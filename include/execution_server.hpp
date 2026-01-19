
#pragma once
#include "config.hpp"
#include "execution_pool.hpp"
#include "socket_server.hpp"
#include <memory>
#include <nlohmann/json.hpp>

/// @breif Execution server that manages the server socket, execution pool and conifg
///        Interface to the executor program
class ExecutionServer
{

  public:
    ExecutionServer(const Config& config);

    void run() { _socket_server->run(); }

  private:
    Config _config;

    std::unique_ptr<ExecutionPool> _execution_pool = nullptr;
    std::unique_ptr<SocketServer>  _socket_server  = nullptr;
    std::mutex                     _socket_mutex;

    std::vector<ExecutionResult> emulated_result(size_t num_tests);
    bool                         validate_json_msg(const nlohmann::json& msg);
};
