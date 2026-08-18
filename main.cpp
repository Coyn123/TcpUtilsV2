//#include "TcpHttpServerImplementation.h"
#include <iostream>
#include <cstdio>
#include "listener.h"

/*int main() {
  TcpHttpServerImplementation test;
  test.run();
  return 0;
  }*/

int main() {
    auto listener = Listener::create(8080);
    if (!listener) {
        return 1;
    }
    printf("listening, fd = %d\n", listener->get());

    //Accept() debug
    for (int i = 1; i <= 3; i = i + 1) {
        printf("[%d] waiting for a client...\n", i);

        std::optional<Connection> conn = listener->accept();
        if (!conn) {
            printf("[%d] accept failed, stopping\n", i);
            break;
        }
        printf("[%d] client connected on fd %d\n", i, conn->get());

    }
    // ^^^ Accept() Debug

    printf("\ndone -- listener closes as main returns\n");
    return 0;
}
