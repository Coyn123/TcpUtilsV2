# TcpUtilsV2

A cross-platform TCP sockets library in C++, built around RAII ownership and explicit `Result<T>` error handling instead of exceptions or raw error codes.

## Status

Active work in progress. Development is currently focused on a small, self-contained core — `Listener` → `Connection` → `IStream` — rather than the full library.

## Active core

- **[Platform.h](Platform.h)** — thin cross-platform shim over BSD sockets / Winsock: a common `socket_t` type, `close_socket`/`last_error` wrappers, one-time `WSAStartup`/`WSACleanup` via `ensure_started()`, and SIGPIPE-safe send flags.
- **[ResultType.h](ResultType.h)** — `tcp::Result<T>`, a minimal success-value-or-error-code type used in place of exceptions across the API.
- **[Stream.h](Stream.h)** — `tcp::IStream`, the `read_some` / `write_some` / `write_all` interface `Connection` implements, so higher-level code depends on an abstraction rather than a raw socket.
- **[Connection.h](Connection.h) / [Connection.cpp](Connection.cpp)** — RAII wrapper around a connected socket: move-only, closes its descriptor on destruction, retries on `EINTR`, and reports peer EOF as `Result::ok(0)` rather than an error.
- **[Listener.h](Listener.h) / [Listener.cpp](Listener.cpp)** — RAII wrapper around a listening socket. `Listener::create(port)` is a factory returning `Result<Listener>`, so a partially-initialized listener can never escape into a live object; `accept()` hands back a `Connection`.
- **[BufferedReader.h](BufferedReader.h) / [BufferedReader.cpp](BufferedReader.cpp)** — delimiter-based buffered reads (`read_until`) on top of an `IStream`. Interface is in place; the implementation is currently a stub.
- **[Main.cpp](Main.cpp)** — a small echo-server demo exercising `Listener` and `Connection` end to end.

Together these give you a working, dependency-free echo server today, with `BufferedReader` as the next piece being built out.

## Direction / work-in-progress area

The `Tcp*`-prefixed files (`TcpUtils`, `TcpHttpProtocol`, `TcpHttpServer`, `TcpHttpServerImplementation`) are an earlier, inheritance-based HTTP server implementation — raw `SOCKET` handles, manual `closesocket` calls, no RAII. They are **not wired into the active core** and are not currently built or used.

They're kept in the repo as the target direction for the new design: the plan is to re-implement the same HTTP server on top of `Connection`, `Listener`, and `BufferedReader` once that core is complete, replacing manual lifetime management with RAII and return codes with `Result<T>`. Treat this area as a reference for where the project is headed, not as usable code.

## Building

There's no build system wired up yet — compile the active core directly:

```bash
# Linux/macOS
g++ -std=c++20 Main.cpp Listener.cpp Connection.cpp BufferedReader.cpp -o server

# Windows (MinGW)
g++ -std=c++20 Main.cpp Listener.cpp Connection.cpp BufferedReader.cpp -o server.exe -lws2_32
```
