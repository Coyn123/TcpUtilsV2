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
        printf("[%d] client connected\n", i);

        char buffer[512];
        tcp::Result<size_t> response = conn.read_some(buffer, sizeof(buffer));

        if (!response.has_value()) {
            fprintf(stderr, "[%d] read failed: %s\n", i, strerror(response.error()));
            continue;
        }

        int bytesRead = (int)response.value();
        printf("[%d] read %d bytes: [%.*s]\n", i, bytesRead, bytesRead, buffer);

        if (bytesRead == 0) {
            printf("[%d] client sent nothing (EOF) -- nothing to echo\n", i);
            continue;
        }

        tcp::Result<size_t> written = conn.write_some(buffer, (size_t)bytesRead);

        if (!written.has_value()) {
            fprintf(stderr, "[%d] write failed: %s\n", i, strerror(written.error()));
            continue;
        }

        int bytesWritten = (int)written.value();
        printf("[%d] echoed %d of %d bytes back to the client\n", i, bytesWritten, bytesRead);

        if (bytesWritten < bytesRead) {
            printf("[%d] (short write -- write_all would need to loop here)\n", i);
        }

    }

    printf("\ndone -- listener closes as main returns\n");
    return 0;
}
