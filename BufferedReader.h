#include "Stream.h"
#include <string>
namespace tcp {
    struct BufferedResult {
        std::string bytes;
        bool complete;
    };
}

class BufferedReader {
    public:
        explicit BufferedReader(tcp::IStream& stream);

        BufferedReader(const BufferedReader&) = delete;
        BufferedReader& operator=(const BufferedReader&) = delete;

        BufferedReader(BufferedReader&&) noexcept = delete;
        BufferedReader& operator=(BufferedReader&&) noexcept = delete;

        tcp::Result<tcp::BufferedResult> read_until(const std::string& delim, size_t byte_max);

    private:
        tcp::IStream& stream_;
        std::string buffer_;

};
