#include "BufferedReader.h"

BufferedReader::BufferedReader(tcp::IStream& stream) : stream_(stream) {}

tcp::Result<tcp::BufferedResult> BufferedReader::read_until(const std::string& delim, size_t byte_max) {
    return tcp::Result<tcp::BufferedResult>::ok({"test", false});
}
