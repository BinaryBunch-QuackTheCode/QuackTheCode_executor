
#pragma once
#include "executor.hpp"
#include "socket_server.hpp"
#include "thread_pool.hpp"
#include <memory>
#include <string>

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
    Executor                      _executor;
    ExecutionThreadPool           _execution_pool;
    std::unique_ptr<SocketServer> _socket_server = nullptr;
};
