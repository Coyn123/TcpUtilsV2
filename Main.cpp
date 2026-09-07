#include <cstdio>
#include "Listener.h"
#include "BufferedReader.h"
#include "ResultType.h"
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

    const std::string delim = "\n";
    const size_t byte_max = 64;

    for (int i = 1; i <= 3; i = i + 1) {
        printf("[%d] waiting for a client...\n", i);

        tcp::Result<Connection> incoming = listener.accept();
        if (!incoming) {
            fprintf(stderr, "[%d] accept failed: %s\n", i, strerror(incoming.error()));
            break;
        }

        Connection conn = std::move(incoming.value());
        printf("[%d] client connected\n", i);

        BufferedReader reader(conn);

        int msg_num = 1;
        while (true) {
            tcp::Result<tcp::BufferedResult> res = reader.read_until(delim, byte_max);

            if (!res.has_value()) {
                fprintf(stderr, "[%d] read_until failed: %s\n", i, strerror(res.error()));
                break;
            }

            const tcp::BufferedResult& br = res.value();
            printf("[%d] msg %d complete=%s (%zu bytes): [%s]\n",
                   i, msg_num, br.complete ? "true" : "false", br.bytes.size(), br.bytes.c_str());
            msg_num++;

            if (!br.complete) {
                printf("[%d] stream ended or byte_max hit before the delimiter showed up -- done with this client\n", i);
                break;
            }
        }
    }

    printf("\ndone -- listener closes as main returns\n");
    return 0;
}
