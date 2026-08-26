//#include "TcpHttpServerImplementation.h"
#include <cstdio>
#include "listener.h"
#include "resultType.h"
#include <cstring>
#include <utility>

int main() {
    uint16_t port = 8080;

    tcp::Result<Listener> made = Listener::create(port);
    if (!made) {
        fprintf(stderr, "create failed: %s\n", strerror(made.error()));
        return 1;
    }

    Listener listener = std::move(made.value());
    printf("listening on %d\n", port);

    for (int i = 1; i <= 3; i = i + 1) {
        printf("[%d] waiting for a client...\n", i);

        tcp::Result<Connection> incoming = listener.accept();
        if (!incoming) {
            fprintf(stderr, "[%d] accept failed: %s\n", i, strerror(incoming.error()));
            break;
        }

        Connection conn = std::move(incoming.value());
        char buffer[512];
        tcp::Result<size_t> response = conn.read_some(buffer, sizeof(buffer));
        if ( response.has_value() ) {
            // size_t to int cast because of printf only
            int value = response.value();
            printf("[%.*s] result\n", value, buffer);
            printf("result value ---> %d\n", value);
        } else {
            fprintf(stderr, "read failed: %s\n", strerror(response.error()));
        }
        printf("[%d] client connected\n", i);

    }   // conn destructs here -> close() on the client fd

    printf("\ndone -- listener closes as main returns\n");
    return 0;
}
