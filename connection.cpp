#include "connection.h"
#include "platform.h"
//fd = file descriptor

Connection::Connection(socket_t fd): fd_(fd) {

}

Connection::~Connection() {
    if(fd_ != kInvalidSocket) {
        close_socket(fd_);
    }
}

Connection::Connection(Connection&& other) noexcept: fd_(other.fd_) {
    other.fd_ = kInvalidSocket;
}

Connection& Connection::operator=(Connection&& other) noexcept {

    socket_t tmp = other.fd_;
    other.fd_ = kInvalidSocket;
    if(fd_ != kInvalidSocket) {
        close_socket(fd_);
    }
    fd_ = tmp;
    return *this;
}

//Getter
socket_t Connection::get() const noexcept {
    return fd_;
}
