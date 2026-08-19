#pragma once
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#else
using socket_t = int;
#endif

class Connection {
    public:
        socket_t get() const noexcept;
        //Explicit socket, no silent int conversion
        explicit Connection(socket_t fd);
        ~Connection();

        Connection(const Connection&) = delete;
        Connection& operator=(const Connection&) = delete;

        Connection(Connection&&) noexcept;
        Connection& operator=(Connection&&) noexcept;

    private:
        socket_t fd_;
};
