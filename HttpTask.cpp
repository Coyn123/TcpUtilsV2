#include "HttpTask.h"
#include "HttpParser.h"
#include <utility>

HttpTask::HttpTask(Connection conn) : connection_(std::move(conn)) {}

void HttpTask::run_task() {
    printf("client connected\n");
    BufferedReader reader(connection_);

    tcp::Result<HttpRequest> request = Http::build_request(reader);
    if (!request) {
        fprintf(stderr, "build_request failed: code %d\n", request.error());
        return;
    }
    const HttpRequest& req = request.value();

    if(req.is_ws_upgrade) {
        //Key guaranteed to exist via build_request.. flag is set.
        std::string socket_key = req.headers.at("sec-websocket-key");
        socket_key += kRFCguid;


    }
    printf("%s %s %s\n",req.method.c_str(), req.url.c_str(), req.version.c_str());

    tcp::Result<HttpResponse> response = Http::build_response(req);
    if (!response) {
        fprintf(stderr, "build_response failed: code %d\n", response.error());
        return;
    }

    std::string bytes = Http::serialize_response(response.value());

    tcp::Result<void> sent = connection_.write_all(bytes.data(), bytes.size());
    if (!sent) {
        fprintf(stderr, "write_all failed: %s\n", strerror(sent.error()));
        return;
    }
    printf("sent %zu bytes (status %d)\n", bytes.size(), response.value().status_code);
}
