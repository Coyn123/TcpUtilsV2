# TcpUtilsV2

A cross-platform TCP sockets library in C++, built around RAII ownership and explicit `Result<T>` error handling instead of exceptions or raw error codes.

## Status

Active work in progress. Development is currently focused on a small, self-contained core, `Listener` → `Connection` → `IStream` → `TaskBase`/`HttpTask`, rather than the full library.

## Active core

- **[src/core/Platform.h](src/core/Platform.h)** — thin cross-platform shim over BSD sockets / Winsock: a common `socket_t` type, `close_socket` wrapper, one-time `WSAStartup`/`WSACleanup` via `ensure_started()`, and SIGPIPE-safe send flags. Two separate last-error accessors — `last_error()` (socket calls; `WSAGetLastError()` on Windows) and `last_file_error()` (everything else; plain `GetLastError()` on Windows) — since the two aren't interchangeable there.
- **[src/core/ResultType.h](src/core/ResultType.h)** — `tcp::Result<T>`, a minimal success-value-or-error-code type used in place of exceptions across the API.
- **[src/transport/Stream.h](src/transport/Stream.h) / [src/transport/Stream.cpp](src/transport/Stream.cpp)** — `tcp::IStream`, the `read_some` / `write_some` / `write_all` interface `Connection` implements, so higher-level code depends on an abstraction rather than a raw socket. `read_some`/`write_some` are pure virtual; `write_all` is a concrete loop over `write_some` that accumulates bytes written and retries on short writes.
- **[src/transport/Connection.h](src/transport/Connection.h) / [src/transport/Connection.cpp](src/transport/Connection.cpp)** — RAII wrapper around a connected socket: move-only, closes its descriptor on destruction, retries on `EINTR`, and reports peer EOF as `Result::ok(0)` rather than an error.
- **[src/transport/Listener.h](src/transport/Listener.h) / [src/transport/Listener.cpp](src/transport/Listener.cpp)** — RAII wrapper around a listening socket. `Listener::create(port)` is a factory returning `Result<Listener>`, so a partially-initialized listener can never escape into a live object; `accept()` hands back a `Connection`.
- **[src/transport/BufferedReader.h](src/transport/BufferedReader.h) / [src/transport/BufferedReader.cpp](src/transport/BufferedReader.cpp)** — buffered reads on top of an `IStream`: `read_until` for delimiter-based reads (request lines, headers), `read_exact` for fixed-size reads (a body sized by `Content-Length`). Both share a private chunk-reading helper.
- **[src/util/Base64.h](src/util/Base64.h) / [src/util/Base64.cpp](src/util/Base64.cpp)** and **[src/util/Sha1.h](src/util/Sha1.h) / [src/util/Sha1.cpp](src/util/Sha1.cpp)** — generic, dependency-free `base64_encode`/`sha1` byte-manipulation utilities, with no HTTP/WebSocket-specific knowledge. Used by the in-progress WebSocket handshake (see Direction below).
- **[src/http/HttpParser.h](src/http/HttpParser.h) / [src/http/HttpParser.cpp](src/http/HttpParser.cpp)** — the HTTP layer, composed over `BufferedReader` rather than a concrete socket:
  - `Http::build_request(reader)` — parses a request line, headers (merging duplicates, rejecting duplicate `Content-Length`/`Host`), and a `Content-Length`-sized body into an `HttpRequest`. Failures return a `Http::ParseError` (distinct from the propagated OS `errno` values used for actual socket failures) via `tcp::Result`.
  - `Http::route(url)` — resolves a URL to a file path via a small static lookup table, falling back to `templates/index.html` for anything unmapped.
  - `Http::build_response(request)` — reads the routed file off disk and builds an `HttpResponse`; a missing file is a normal `200`/`404`-style `Result::ok`, not an `err` — `err` is reserved for genuine I/O failures, reported via `last_file_error()`.
  - `Http::serialize_response(response)` — renders an `HttpResponse` into wire-format bytes, with a small status-code → reason-phrase lookup (`200`/`404` today).
- **[src/tasks/TaskBase.h](src/tasks/TaskBase.h)**: `TaskBase`, an abstract one-shot unit of work, a virtual destructor (so ownership through a base handle cleans up correctly) plus a single pure virtual `run_task()`. No data of its own; each concrete task owns whatever it needs.
- **[src/tasks/HttpTask.h](src/tasks/HttpTask.h) / [src/tasks/HttpTask.cpp](src/tasks/HttpTask.cpp)**: `HttpTask`, the concrete `TaskBase` for one accepted connection. Takes ownership of a `Connection` at construction; `run_task()` runs the `build_request` → `build_response` → `serialize_response` → `write_all` sequence exactly once and returns. The connection closes via `Connection`'s own destructor once the task itself is destroyed.
- **[src/Main.cpp](src/Main.cpp)**: accepts connections in a loop, wraps each one in a `unique_ptr<TaskBase>` (`HttpTask`), and runs it on a `std::thread`. That thread is joined immediately for now, so this proves the task/thread wiring works; it isn't real concurrency yet (see Direction below).

Together these give you a working, dependency-free HTTP server today, serving static files out of `templates/` with basic routing. The transport core — `Listener`, `Connection`, `IStream`/`Stream`, `BufferedReader` — is feature-complete for basic reads and writes.

## Direction

The HTTP layer is minimal by design right now — known gaps, not oversights:
- Only `templates/index.html` and whatever's manually added to `Http::route`'s lookup table are servable; there's no directory serving or MIME-type-by-extension yet (`Content-Type` isn't set on responses at all currently).
- The reason-phrase table in `serialize_response` only knows `200`/`404`; anything else renders as `"Unknown"`.
- No keep-alive: each `HttpTask` runs once and ends.
- Concurrency is proof-of-concept only: `Main.cpp` spawns one thread per connection and joins it immediately, so requests are still handled one at a time. Next step is a bounded worker pool, a fixed set of threads pulling `unique_ptr<TaskBase>` off a shared queue guarded by a condition variable and mutex, instead of spawning a thread per connection.

Longer-term, an HTTPS layer should be able to slot in as a `TlsStream : IStream` without changing the HTTP code at all, since everything above is composed over `IStream`/`BufferedReader` rather than bound to a concrete transport.

An earlier, inheritance-based HTTP server implementation (`TcpUtils`, `TcpHttpProtocol`, `TcpHttpServer`, `TcpHttpServerImplementation` — raw `SOCKET` handles, manual `closesocket` calls, no RAII) was removed from the working tree since it was never wired into the active core. It's still recoverable from git history if it's ever worth referencing again.

## Building

There's no build system wired up yet — compile the active core directly. Source lives under `src/`, organized by abstraction layer (`core/`, `transport/`, `http/`, `util/`, `tasks/`, plus `websocket/` once the frame module lands); `-Isrc` lets every file `#include` its dependencies by layer-qualified path (e.g. `#include "transport/Connection.h"`) without needing a flag per folder.

```bash
# Linux/macOS
g++ -std=c++20 -Isrc src/Main.cpp src/transport/Listener.cpp src/transport/Connection.cpp src/transport/Stream.cpp src/transport/BufferedReader.cpp src/http/HttpParser.cpp src/tasks/HttpTask.cpp src/util/Base64.cpp src/util/Sha1.cpp -o server -pthread

# Windows (MinGW)
g++ -std=c++20 -Isrc src/Main.cpp src/transport/Listener.cpp src/transport/Connection.cpp src/transport/Stream.cpp src/transport/BufferedReader.cpp src/http/HttpParser.cpp src/tasks/HttpTask.cpp src/util/Base64.cpp src/util/Sha1.cpp -o server.exe -lws2_32
```

`std::thread` is used now. If the Windows build fails to link, or `std::thread` throws at runtime, add `-pthread` (needed on some MinGW-w64 builds; a "win32 threads" MinGW distribution doesn't support `std::thread` at all and needs a different toolchain).

## Running

```bash
./server
```

Listens on port `8080` and serves requests until `accept()` fails. Visit `http://localhost:8080/` (or `curl -v`) to hit `templates/index.html`.
