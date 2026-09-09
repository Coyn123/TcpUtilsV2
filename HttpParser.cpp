#include "HttpParser.h"
#include "Platform.h"
#include "Connection.h"
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <cctype>
#include <limits>

tcp::Result<HttpRequest> Http::build_request(BufferedReader& reader) {

    HttpRequest ret;
    tcp::Result<tcp::BufferedResult> try_read = reader.read_until("\r\n", kMaxRequestLineBytes);

    if (!try_read) {
        return tcp::Result<HttpRequest>::err(try_read.error());
    } else if (!try_read.value().complete) {
        return tcp::Result<HttpRequest>::err(static_cast<int>(Http::ParseError::RequestLineTooLong));
    }
    std::string entire_request = try_read.value().bytes;
    std::stringstream ss(entire_request);

    std::vector<std::string> split_request((std::istream_iterator<std::string>(ss)), std::istream_iterator<std::string>());
    if (split_request.size() != 3) {
        return tcp::Result<HttpRequest>::err(static_cast<int>(Http::ParseError::RequestLineMalformed));
    }

    ret.method = split_request[0];
    ret.url = split_request[1];
    ret.version = split_request[2];


    for(;;) {
        tcp::Result<tcp::BufferedResult> try_headers = reader.read_until("\r\n", kMaxRequestLineBytes);
        if(!try_headers) {
            return tcp::Result<HttpRequest>::err(try_headers.error());
        } else if (!try_headers.value().complete) {
            return tcp::Result<HttpRequest>::err(static_cast<int>(Http::ParseError::HeaderLineTooLong));
        }
        if (try_headers.value().bytes == "\r\n") break;

        //Find
        std::string cur = try_headers.value().bytes;
        size_t splitter = cur.find(":");
        if (splitter == std::string::npos) return tcp::Result<HttpRequest>::err(static_cast<int>(Http::ParseError::HeaderMissingColon));
        size_t start = cur.find_first_not_of(" ", splitter + 1);
        std::string value = cur.substr(start, (cur.size()-2) - start);
        std::string key = cur.substr(0, splitter);

        //Sanitize
        auto existing = ret.headers.find(key);
        if (existing != ret.headers.end()) {
            //Content-Length / Host duplicates are dangerous, every other header can be merged per RFC 7230 3.2.2
            if (key == "Content-Length" || key == "Host")
                return tcp::Result<HttpRequest>::err(static_cast<int>(Http::ParseError::DuplicateHeader));
            existing->second += ", " + value;
        } else {
            ret.headers[key] = value;
        }
    }

    auto check_length = ret.headers.find("Content-Length");

    //If we have a Content-Length at all (start)
    if (check_length != ret.headers.end() ) {

        std::string clen = check_length->second;

        if(clen.size() == 0) return tcp::Result<HttpRequest>::err(static_cast<int>(Http::ParseError::ContentLengthEmpty));
        if(clen.size() > std::numeric_limits<unsigned long>::digits10) {
            return tcp::Result<HttpRequest>::err(static_cast<int>(Http::ParseError::ContentLengthTooLong));
        }

        for (const char& c : clen) {
            if(!std::isdigit(static_cast<unsigned char>(c))) {
                return tcp::Result<HttpRequest>::err(static_cast<int>(Http::ParseError::ContentLengthInvalid));
            }
        }
        unsigned long rclen = std::stoul(clen, nullptr, 10);

        //Is the real content length still 0?
        if(rclen != 0) {
            tcp::Result<tcp::BufferedResult> try_body = reader.read_exact(rclen);

            if(!try_body) {
                return tcp::Result<HttpRequest>::err(try_body.error());
            } else if (!try_body.value().complete) {
                return tcp::Result<HttpRequest>::err(static_cast<int>(Http::ParseError::BodyTruncated));
            }
            ret.body = try_body.value().bytes;
        }

    } // If we have a Content-Length at all (end)

    //Finally, a finished request!
    return tcp::Result<HttpRequest>::ok(ret);

}
