#pragma once
#include <istream>
#include <functional>

#ifdef _WIN32
#include <ws2tcpip.h>
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
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
