# TcpUtilsV2

A cross-platform TCP sockets library in C++, built around RAII ownership and explicit `Result<T>` error handling instead of exceptions or raw error codes.

## Status

Active work in progress. Development is currently focused on a small, self-contained core — `Listener` → `Connection` → `IStream` — rather than the full library.

## Active core

- **[Platform.h](Platform.h)** — thin cross-platform shim over BSD sockets / Winsock: a common `socket_t` type, `close_socket`/`last_error` wrappers, one-time `WSAStartup`/`WSACleanup` via `ensure_started()`, and SIGPIPE-safe send flags.
- **[ResultType.h](ResultType.h)** — `tcp::Result<T>`, a minimal success-value-or-error-code type used in place of exceptions across the API.
- **[Stream.h](Stream.h) / [Stream.cpp](Stream.cpp)** — `tcp::IStream`, the `read_some` / `write_some` / `write_all` interface `Connection` implements, so higher-level code depends on an abstraction rather than a raw socket. `read_some`/`write_some` are pure virtual; `write_all` is a concrete loop over `write_some` that accumulates bytes written and retries on short writes.
- **[Connection.h](Connection.h) / [Connection.cpp](Connection.cpp)** — RAII wrapper around a connected socket: move-only, closes its descriptor on destruction, retries on `EINTR`, and reports peer EOF as `Result::ok(0)` rather than an error.
- **[Listener.h](Listener.h) / [Listener.cpp](Listener.cpp)** — RAII wrapper around a listening socket. `Listener::create(port)` is a factory returning `Result<Listener>`, so a partially-initialized listener can never escape into a live object; `accept()` hands back a `Connection`.
- **[BufferedReader.h](BufferedReader.h) / [BufferedReader.cpp](BufferedReader.cpp)** — buffered reads on top of an `IStream`: `read_until` for delimiter-based reads (request lines, headers), `read_exact` for fixed-size reads (a body sized by `Content-Length`). Both share a private chunk-reading helper.
- **[Main.cpp](Main.cpp)** — a small echo-server demo exercising `Listener` and `Connection` end to end.

Together these give you a working, dependency-free echo server today. The active core — `Listener`, `Connection`, `IStream`/`Stream`, `BufferedReader` — is now feature-complete for basic reads and writes.

## Direction

Building an HTTP layer on top of `Connection`, `Listener`, and `BufferedReader` — request/response parsing composed over `IStream` rather than bound to a concrete transport, so an HTTPS layer can later slot in as a `TlsStream : IStream` without changing the HTTP code at all.

An earlier, inheritance-based HTTP server implementation (`TcpUtils`, `TcpHttpProtocol`, `TcpHttpServer`, `TcpHttpServerImplementation` — raw `SOCKET` handles, manual `closesocket` calls, no RAII) was removed from the working tree since it was never wired into the active core. It's still recoverable from git history if it's ever worth referencing again.

## Building

There's no build system wired up yet — compile the active core directly:

```bash
# Linux/macOS
g++ -std=c++20 Main.cpp Listener.cpp Connection.cpp Stream.cpp BufferedReader.cpp -o server

# Windows (MinGW)
g++ -std=c++20 Main.cpp Listener.cpp Connection.cpp Stream.cpp BufferedReader.cpp -o server.exe -lws2_32
```
