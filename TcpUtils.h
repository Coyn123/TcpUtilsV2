#pragma once
#include <unordered_map>
#include <string>
#include <functional>
#include <winsock2.h>

class TcpUtils {
  protected:
    std::unordered_map<std::string, std::string> mime_types;
    SOCKET server_fd;
    int port;
  
  public:
    TcpUtils(int port = 6969);
    ~TcpUtils();
    
    void initialize_mime_types();
    
    std::string get_current_time();
    std::string get_file_content(const std::string& path);
   
    SOCKET create_connection();
    void send_data(SOCKET client_sock, const std::string& data);
    std::string receive_data(SOCKET conn);

};
