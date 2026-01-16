

#pragma once

#include <string> 
#include <stdint.h> 
#include <optional> 

enum class SocketType
{
    UNIX,
    TCP
};

struct UnixSocketArgs
{
    std::string path; 
};

struct TCPSocketArgs
{
    std::string ip_addr; 
    uint16_t port; 
};

struct Config
{

    SocketType socket_type; 

    std::optional<UnixSocketArgs> unix_args;  

    std::optional<TCPSocketArgs> tcp_args;  

    std::string sandbox_cfg_path; 
};


Config parse_config(const std::string& config_path); 
