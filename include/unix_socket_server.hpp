
#pragma once

#include "socket_server.hpp"

/// @breif Unix Socket implementation of socket server. Used when the executor 
///        is on the same machine as the game server 
class UnixSocketServer : public SocketServer
{

  public:
    UnixSocketServer(const std::string& socket_path);

    ~UnixSocketServer() override; 

  private:
    int create_socket_fd() override; 
    const std::string _socket_path; 
};
