
#pragma once

#include "socket_server.hpp"

class TCPSocketServer : public SocketServer
{

  public:
    TCPSocketServer(const std::string& ip_address, uint16_t port);

  private:
    int create_socket_fd() override; 
};
