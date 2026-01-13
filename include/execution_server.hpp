
#pragma once
#include <string>
#include <memory> 
#include "socket_server.hpp"
#include "executor.hpp"

class ExecutionServer
{

public: 
    enum class SocketType
    {
        UNIX 
    };

    struct Args
    {
        SocketType  socket_type; 
        std::string socket_path; 
        std::string jail_config_path; 
    };


    ExecutionServer(const Args& args);

    void run() { _socket_server->run(); }


private: 
    Executor _executor; 
    std::unique_ptr<SocketServer> _socket_server = nullptr; 

};

