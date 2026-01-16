

#include "tcp_socket_server.hpp"
#include <sys/socket.h> 
#include <arpa/inet.h> 


int TCPSocketServer::create_socket_fd()
{
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) 
    {
        throw std::system_error(errno, std::generic_category(), "socket failed");
    }
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(_port); 
    if (inet_pton(AF_INET, _ip_addr.c_str(), &(addr.sin_addr)) <= 0)
    {
        throw std::runtime_error("Invalid IP address");
    }

    if (bind(socket_fd, (sockaddr*)&addr, sizeof(addr)) < 0)
    {
        throw std::system_error(errno, std::generic_category(), "bind failed");
    }

    listen(socket_fd, SOMAXCONN);
    return socket_fd; 
}



