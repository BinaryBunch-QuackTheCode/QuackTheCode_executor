

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

enum class ExecutionPoolType
{
    THREAD_POOL, 
    EMULATED, 
    REDIS
};

/// @breif Configuration for the execution server.  
struct Config
{
    ExecutionPoolType execution_pool_type; 

    // Byte stream socket type 
    SocketType socket_type; 

    std::optional<UnixSocketArgs> unix_args;  

    std::optional<TCPSocketArgs> tcp_args;  

    // Path to the nsjail sandbox config 
    std::string sandbox_cfg_path; 

    uint32_t num_threads; 

};


Config parse_config(const std::string& config_path); 
