#pragma once
#include "platform.h"

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
