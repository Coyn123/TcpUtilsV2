#include <string>
#include <unordered_map>
#include <vector>
#include "BufferedReader.h"
struct HttpResponse {
    int status_code = 0;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
};

struct HttpRequest {
    std::string url;
    std::string method;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
    std::string version;

};

namespace Http {
    tcp::Result<HttpRequest> build_request(BufferedReader& reader);

};
