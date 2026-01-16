

#include "unix_socket_server.hpp"
#include <sys/socket.h> 
#include <system_error> 
#include <sys/un.h> 


UnixSocketServer::UnixSocketServer(const std::string& socket_path) : _socket_path(socket_path)
{}

int UnixSocketServer::create_socket_fd()
{
    unlink(_socket_path.c_str());
    int socket_fd = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (socket_fd < 0) 
    {
        throw std::system_error(errno, std::generic_category(), "socket failed");
    }

    sockaddr_un addr; 
    addr.sun_family = AF_UNIX; 
    strcpy(addr.sun_path, _socket_path.c_str());

    if (bind(socket_fd, (sockaddr*)&addr, sizeof(addr)) < 0)
    {
        throw std::system_error(errno, std::generic_category(), "bind failed");
    }

    listen(socket_fd, SOMAXCONN);
    return socket_fd;
}

UnixSocketServer::~UnixSocketServer()
{
    unlink(_socket_path.c_str());
}


