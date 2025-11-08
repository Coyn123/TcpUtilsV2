#pragma once
#include "TcpServer.h"
#include <string>
#include <unordered_map>

#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#define SOCKET int
#endif

class TcpHttpServer : public TcpServer {
  protected:
    std::unordered_map<std::string, std::function<std::string(const std::string&)>> get_routes;
    //std::unordered_map<std::string, std::string> post_routes;
  public:
    TcpHttpServer();
    //~TcpHttpServer();

    std::string handle_get_request(const std::string& path);
    //std::string handle_post_requests(const std::string& body);
    std::string get_mime_type(const std::string& path);

    void initialize_routes();
    void handle_client(SOCKET client_sock);
    void run();
};
