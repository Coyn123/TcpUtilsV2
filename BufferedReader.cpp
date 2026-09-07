#include "BufferedReader.h"
#include "Platform.h"
#include "ResultType.h"
#include <algorithm>

BufferedReader::BufferedReader(tcp::IStream& stream) : stream_(stream) {}

std::string BufferedReader::split_on_delim(std::string& txt, size_t pos, const std::string& delim) {
    std::string ret = txt.substr(0, pos + delim.size());
    buffer_ = txt.substr(pos + delim.size());
    return ret;
}

tcp::Result<tcp::BufferedResult> BufferedReader::read_until(const std::string& delim, size_t byte_max) {
    char chunk[4096];
    std::string txt = buffer_;
    size_t byte_check = txt.size();
    buffer_.clear();
    size_t pos = txt.find(delim);

    if (pos != std::string::npos) {
        std::string ret = split_on_delim(txt, pos, delim);
        return tcp::Result<tcp::BufferedResult>::ok({ret, true});
    }
    while(byte_check < byte_max && pos == std::string::npos) {
        tcp::Result<size_t> out = stream_.read_some( chunk, std::min(byte_max - byte_check, sizeof(chunk)) );
        if(!out.has_value()) {
            return tcp::Result<tcp::BufferedResult>::err(out.error());
        }
        size_t out_n = out.value();
        if(out_n == 0) {
            return tcp::Result<tcp::BufferedResult>::ok({txt, false});
        }
        txt.append(chunk, out_n);
        byte_check += out_n;
        pos = txt.find(delim);
        if (pos != std::string::npos) {
            std::string ret = split_on_delim(txt, pos, delim);
            return tcp::Result<tcp::BufferedResult>::ok({ret, true});
        }
    }
    return tcp::Result<tcp::BufferedResult>::ok({txt, false});

}
