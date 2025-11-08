#pragma once
#include "TcpUtils.h"

#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#define SOCKET int
#endif

class TcpServer : public TcpUtils {
  protected:
    std::unordered_map<std::string, std::string> routes;

  public:
    void start_listening(SOCKET conn);

    virtual void initialize_routes() = 0;
    virtual void handle_client(SOCKET client_sock) = 0;
    virtual void run() = 0;
};
