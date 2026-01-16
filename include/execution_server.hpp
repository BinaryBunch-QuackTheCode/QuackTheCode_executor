
#pragma once
#include "config.hpp"
#include "executor.hpp"
#include "socket_server.hpp"
#include "thread_pool.hpp"
#include <memory>

class ExecutionServer
{

  public:
    ExecutionServer(const Config& config);

    void run() { _socket_server->run(); }

  private:
    Config _config;

    Executor                      _executor;
    ExecutionThreadPool           _execution_pool;
    std::unique_ptr<SocketServer> _socket_server = nullptr;
};
