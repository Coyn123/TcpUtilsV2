#include "WSTask.h"
#include "websocket/WebSocketFrame.h"
#include "transport/BufferedReader.h"
#include <utility>

WSTask::WSTask(Connection conn) : connection_(std::move(conn)) {}


void WSTask::run_task() {
    printf("WebSocket uprade established\n");

    BufferedReader reader(connection_);

    for(;;) {
        tcp::Result<WsFrame> try_parse = WebSocket::parse_frame(reader);
        if (!try_parse) {
            fprintf(stderr, "WS parse failed: code %d\n", try_parse.error());
            return;
        }
        size_t opcode_ = try_parse.value().opcode;

        switch(opcode_)
        {
            //Text
            case(0x1):
                {
                    WsFrame ret;
                    ret.finbit = true;
                    ret.opcode = 0x1;
                    ret.payload = try_parse.value().payload;

                    std::string out = WebSocket::serialize_frame(ret);
                    tcp::Result<void> try_write = connection_.write_all(out.data(), out.size());
                    if(!try_write) {
                        fprintf(stderr, "Connection write error failed: code %d\n", try_write.error());
                    }
                }
                break;

            //Binary
            case(0x2):
                {
                    WsFrame ret;
                    ret.finbit = true;
                    ret.opcode = 0x2;
                    ret.payload = try_parse.value().payload;

                    std::string out = WebSocket::serialize_frame(ret);
                    tcp::Result<void> try_write = connection_.write_all(out.data(), out.size());
                    if(!try_write) {
                        fprintf(stderr, "Connection write error failed: code %d\n", try_write.error());
                    }
                }
                break;
            //Close
            case(0x8):
                    {
                        WsFrame ret;
                        ret.finbit = true;
                        ret.opcode = 0x8;
                        ret.payload = try_parse.value().payload;

                        std::string out = WebSocket::serialize_frame(ret);
                        tcp::Result<void> try_write = connection_.write_all(out.data(), out.size());
                        if(!try_write) {
                            fprintf(stderr, "Connection write error failed: code %d\n", try_write.error());
                        }
                    }
                    return;
            //Ping
            case(0x9):
                {
                    WsFrame ret;
                    ret.finbit = true;
                    ret.opcode = 0xA;
                    ret.payload = try_parse.value().payload;

                    std::string out = WebSocket::serialize_frame(ret);
                    tcp::Result<void> try_write = connection_.write_all(out.data(), out.size());
                    if(!try_write) {
                        fprintf(stderr, "Connection write error failed: code %d\n", try_write.error());
                    }
                }
                break;
            //Pong
            case(0xA):
                break;
        }
    }
    return;
}
