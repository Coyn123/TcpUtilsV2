#include "listener.h"
#include <cstdint>
#include <cstdio>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

std::optional<Listener> Listener::create(uint16_t port) {

    socket_t fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    //Always close fd as no deconstructor owns the cleanup here, we are creating a static factory for objects,
    //We still own the fd value. Benefit of the factory: errors out to no object, instead of a partial object with incomplete values
    //After the object is created the deconstructor owns the fd cleanup

    //Check socket validity
    if (fd == -1) {
        perror("socket");
        return std::nullopt;
    }

    //Socket configuartion validity
    int yes = 1;

    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        perror("setsockopt");
        close(fd);
        return std::nullopt;
    }

    //Socket address validity
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    if(bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1) {
        perror("bind");
        close(fd);
        return std::nullopt;
    }

    //Listener + max connection validity
    if(listen(fd, SOMAXCONN) == -1) {
        perror("listen");
        close(fd);
        return std::nullopt;
    }

    return Listener(fd);
}

std::optional<Connection> Listener::accept() {
    socket_t fd = ::accept(fd_, nullptr, nullptr);
    if (fd == -1) {
        perror("accept");
        return std::nullopt;
    }
    return Connection(fd);
}

Listener::~Listener() {
    if (fd_ != -1) {
        close(fd_);
    }
}

Listener::Listener(Listener&& other) noexcept: fd_(other.fd_) {
    other.fd_ = -1;
}

Listener::Listener(socket_t fd): fd_(fd) {}

Listener& Listener::operator=(Listener&& other) noexcept {

    socket_t tmp = other.fd_;
    other.fd_ = -1;
    if(fd_ != -1) {
        close(fd_);
    }
    fd_ = tmp;
    return *this;
}

socket_t Listener::get() const noexcept {
    return fd_;
}
