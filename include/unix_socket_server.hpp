
#pragma once

#include "socket_server.hpp"

class UnixSocketServer : public SocketServer
{

  public:
    UnixSocketServer(const std::string& socket_path);

    ~UnixSocketServer() override; 

  private:
    int create_socket_fd() override; 
    const std::string _socket_path; 
};
