#include "TcpHttpServer.h"
#include <iostream>

int main() {
  try {
    TcpHttpServer server;
    server.run();
  } catch (const std::exception& e) {
    std::cerr << "Server error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
