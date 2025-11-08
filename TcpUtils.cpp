#include "TcpUtils.h"
#include <fstream>
#include <istream>
#include <ctime>
#include <iostream>

// Possible extras
#ifdef _WIN32
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

TcpUtils::TcpUtils(int port) : port(port) {
#ifdef _WIN32
  WSADATA wsaData;
  WSAStartup(MAKEWORD(2,2), &wsaData);
#endif
}

TcpUtils::~TcpUtils() {
  closesocket(server_fd);
#ifdef _WIN32
  WSACleanup();
#endif
}

std::string TcpUtils::get_current_time() {
  time_t now = time(0);
  struct tm *tstruct = localtime(&now);
  char buf[80];
  strftime(buf, sizeof(buf), "%y-%m-%d %x", tstruct);
  return buf;
}

std::string TcpUtils::get_file_content(const std::string& path) {
#ifdef _WIN32
  std::string full_path = "./templates/" + path;
#else
  std::string full_path = "./templates/" + path;
#endif
  std::ifstream file(full_path, std::ios::binary);
  if (!file) return "";
  return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

void TcpUtils::initialize_mime_types() {
  mime_types[".html"] = "text/html";
  mime_types[".css"] = "text/css";
  mime_types[".js"] = "application/javascript";
  mime_types[".png"] = "image/png";
  mime_types[".jpg"] = "image/jpeg";
  mime_types[".gif"] = "image/gif";
  mime_types[".svg"] = "image/svg+xml";
  mime_types[".json"] = "application/json";
}

SOCKET TcpUtils::create_connection() {
  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  sockaddr_in server_addr{};
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port);

  bind(server_fd, (sockaddr*)&server_addr, sizeof(server_addr));
  
  return server_fd;
}

void TcpUtils::send_data(SOCKET conn, const std::string& data) {
  send(conn, data.c_str(), data.size(), 0);
  return;
}

std::string TcpUtils::receive_data(SOCKET conn) {
  std::string data;
  char buffer[4096];
  int read_bytes = recv(conn, buffer, sizeof(buffer) - 1, 0);
  if (read_bytes <= 0) {
    closesocket(conn);
    return "No Data";
  }

  data.append(buffer, read_bytes);

  return data;
}
