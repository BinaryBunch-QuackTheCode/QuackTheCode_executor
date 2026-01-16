

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

/// @breif Configuration for the execution server.  
struct Config
{

    // Byte stream socket type 
    SocketType socket_type; 

    std::optional<UnixSocketArgs> unix_args;  

    std::optional<TCPSocketArgs> tcp_args;  

    // Path to the nsjail sandbox config 
    std::string sandbox_cfg_path; 

    uint32_t num_threads; 

    // Determines whether user code is actually ran or not 
    bool is_emulated = false; 
};


Config parse_config(const std::string& config_path); 
