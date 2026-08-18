#pragma once
#include "connection.h"
#include <cstdint>
#include <optional>

class Listener {
    public:
        static std::optional<Listener> create(uint16_t port);
        std::optional<Connection> accept();

        ~Listener();
        Listener(const Listener&) = delete;
        Listener& operator=(const Listener&) = delete;
        Listener(Listener&&) noexcept;
        Listener& operator=(Listener&&) noexcept;

        socket_t get() const noexcept;

    private:
        explicit Listener(socket_t fd);
        socket_t fd_;
};
