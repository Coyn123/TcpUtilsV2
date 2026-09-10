#pragma once
#include "transport/BufferedReader.h"
#include <string>

struct WsFrame {
    bool finbit = 0;
    size_t opcode = 0;
    std::string payload;
};

namespace WebSocket {

    tcp::Result<WsFrame> parse_frame(BufferedReader& reader);
    std::string serialize_frame(const WsFrame&);

};
