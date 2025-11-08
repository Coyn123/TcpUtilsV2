#include "TcpHttpServer.h"
#include <string>
#include <sstream>
#include <iostream>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#define SOCKET int
#endif

TcpHttpServer::TcpHttpServer() {
  initialize_routes();
  initialize_mime_types();
}

std::string TcpHttpServer::get_mime_type(const std::string& path) {
  size_t dot_pos = path.find_last_of(".");
  if (dot_pos != std::string::npos) {
    std::string ext = path.substr(dot_pos);
    auto it = mime_types.find(ext);
    if (it != mime_types.end()) return it->second;
  }
  return "application/octet-stream";
}

std::string TcpHttpServer::handle_get_request(const std::string& path) {
  std::string file_path = (path == "/") ? "index.html" : path;
  auto route = get_routes.find(file_path);
  if (route != get_routes.end()) return route->second(file_path);

  std::string content = get_file_content(file_path);
  if (content.empty()) return "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nContent-Length: 9\r\n\r\nNot Found";

  std::ostringstream response;
  response << "HTTP/1.1 200 OK\r\n"
    << "Content-Type: " << this->get_mime_type(file_path) << "\r\n"
    << "Content-Length: " << content.size() << "\r\n\r\n"
    << content;
  return response.str();
}

void TcpHttpServer::initialize_routes() {
  get_routes["/"] = [this](const std::string&) { return handle_get_request("/index.html"); };
}

void TcpHttpServer::handle_client(SOCKET conn) {
  std::string data_in = receive_data(conn);
  std::istringstream request_stream(data_in);
  std::string method, path, version;
  request_stream >> method >> path >> version;

  std::string response;
  if (method == "GET") {
    response = handle_get_request(path);
  } else {
    response = "HTTP/1.1 405 Method Not Allowed\r\nContent-Type: text/plain\r\n\r\nMethod not allowed";
  }
  send_data(conn, response);
  closesocket(conn);
}

void TcpHttpServer::run() {
  SOCKET conn = create_connection();
  start_listening(conn);

  while (true) {
    sockaddr_in client_addr;
    socklen_t client_size = sizeof(client_addr);
    SOCKET client_socket = accept(server_fd, (sockaddr*)&client_addr, &client_size);
    handle_client(client_socket);
  }
}
