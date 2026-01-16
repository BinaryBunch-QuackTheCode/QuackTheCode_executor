
#pragma once

#include "socket_server.hpp"

/// @breif TCP implementation of the socket server. 
///        Used if the executor runs on a seperate machine
class TCPSocketServer : public SocketServer
{

  public:
    /// @breif Create a TCP socket server 
    /// @param ip_address IP addr to listen on
    /// @param port Port to listen on 
    TCPSocketServer(const std::string& ip_address, uint16_t port) : _ip_addr(ip_address), _port(port) {} 

  private:
    std::string _ip_addr; 
    uint16_t _port; 

    int create_socket_fd() override; 
};
