
#pragma once

#include "socket_server.hpp"

class TCPSocketServer : public SocketServer
{

  public:
    TCPSocketServer(const std::string& ip_address, uint16_t port) : _ip_addr(ip_address), _port(port) {} 

  private:
    std::string _ip_addr; 
    uint16_t _port; 

    int create_socket_fd() override; 
};
