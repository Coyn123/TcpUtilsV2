#include "TcpServer.h"
#include <iostream>

void TcpServer::start_listening(SOCKET conn) {
  listen(conn, SOMAXCONN);
  std::cout << "Server listening on port " << port << std::endl;
}
