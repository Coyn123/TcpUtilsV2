#include <cstdio>
#include <cstring>
#include <memory>
#include <thread>
#include <utility>
#include "tasks/HttpTask.h"
#include "transport/Listener.h"
#include "core/ResultType.h"
#include "tasks/TaskBase.h"

int main() {
    uint16_t port = 8080;
    tcp::Result<Listener> made = Listener::create(port);
    if (!made) {
        fprintf(stderr, "create failed: %s\n", strerror(made.error()));
        return 1;
    }

    Listener listener = std::move(made.value());
    printf("listening on %d\n", port);

    for (;;) {
        printf("waiting for a client...\n");

        tcp::Result<Connection> incoming = listener.accept();
        if (!incoming) {
            fprintf(stderr, "accept failed: %s\n", strerror(incoming.error()));
            break;
        }

        Connection conn = std::move(incoming.value());
        printf("client connected\n");

        std::unique_ptr<TaskBase> task = std::make_unique<HttpTask>(std::move(conn));

        // std::thread moves its callable into its own storage instead of going
        // through std::function, so a lambda holding a move-only unique_ptr
        // works here with no wrapper needed.
        std::thread worker([t = std::move(task)]() { t->run_task(); });
        worker.join(); // PoC only: proves the task runs, adds no concurrency yet
    }

    printf("\ndone -- listener closes as main returns\n");
    return 0;
}
