#include <cstdio>
#include "Listener.h"
#include "HttpParser.h"
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

    for (int i = 1; ; i = i + 1) {
        printf("[%d] waiting for a client...\n", i);

        tcp::Result<Connection> incoming = listener.accept();
        if (!incoming) {
            fprintf(stderr, "[%d] accept failed: %s\n", i, strerror(incoming.error()));
            break;
        }

        Connection conn = std::move(incoming.value());
        printf("[%d] client connected\n", i);

        BufferedReader reader(conn);

        tcp::Result<HttpRequest> request = Http::build_request(reader);
        if (!request) {
            fprintf(stderr, "[%d] build_request failed: code %d\n", i, request.error());
            continue;
        }
        const HttpRequest& req = request.value();
        printf("[%d] %s %s %s\n", i, req.method.c_str(), req.url.c_str(), req.version.c_str());

        tcp::Result<HttpResponse> response = Http::build_response(req);
        if (!response) {
            fprintf(stderr, "[%d] build_response failed: code %d\n", i, response.error());
            continue;
        }

        std::string bytes = Http::serialize_response(response.value());

        tcp::Result<void> sent = conn.write_all(bytes.data(), bytes.size());
        if (!sent) {
            fprintf(stderr, "[%d] write_all failed: %s\n", i, strerror(sent.error()));
            continue;
        }

        printf("[%d] sent %zu bytes (status %d)\n", i, bytes.size(), response.value().status_code);
    }

    printf("\ndone -- listener closes as main returns\n");
    return 0;
}
