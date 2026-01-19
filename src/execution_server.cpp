
#include "execution_server.hpp"
#include "executor.hpp"
#include "execution_thread_pool.hpp"
#include "execution_emulated_pool.hpp"
#include "unix_socket_server.hpp"
#include "tcp_socket_server.hpp"

#include <nlohmann/json.hpp>

#include <arpa/inet.h>
#include <sys/socket.h>

using json = nlohmann::json;

ExecutionServer::ExecutionServer(const Config& config) : _config(config){
    switch (config.socket_type)
    {
    case SocketType::UNIX:
        _socket_server = std::make_unique<UnixSocketServer>(config.unix_args->path);
        break;
    case SocketType::TCP: 
        _socket_server = std::make_unique<TCPSocketServer>(config.tcp_args->ip_addr, config.tcp_args->port);
        break;
    default:
        throw std::invalid_argument("Unsupported socket type");
    }

    switch (config.execution_pool_type)
    {
        case ExecutionPoolType::THREAD_POOL: 
            _execution_pool = std::make_unique<ExecutionThreadPool>(config.sandbox_cfg_path, config.num_threads);
            break;
        case ExecutionPoolType::EMULATED: 
            _execution_pool = std::make_unique<ExecutionEmulatedPool>(); 
            break;
        default: 
            throw std::invalid_argument("Unsupported execution pool type");
    }

    _execution_pool->on_execution_complete(
        [this](json message, std::vector<ExecutionResult> results) 
        {
            json results_json = json::array(); 
            for (const auto& result : results)
            {
                results_json.push_back({
                    {"cpu_time_ms",         result.cpu_time_ms},
                    {"stdout",              std::move(result.stdout)}, 
                    {"stderr",              std::move(result.stderr)}, 
                    {"succeeded",           result.succeeded},
                    {"time_limit_exceeded", result.time_limit_exceeded},
                    {"tests_failed",        result.tests_failed}, 
                    {"unknown_error",       result.unknown_error},
                });
            };

            json response = {
                {"status",    "OK"},
                {"game_id",   message["game_id"]},  
                {"player_id", message["player_id"]}, 
                {"results",   std::move(results_json)}
            };

            std::lock_guard<std::mutex> lock(_socket_mutex);
            _socket_server->send(response.dump() + '\n');
        });

    _socket_server->on_recv(
        [this](json message)
        {
            if (!validate_json_msg(message))
            {
                json bad_msg = {
                    { "status", "ERROR", },
                    { "message", "Invalid parameters in JSON message"}
                };
                std::lock_guard<std::mutex> lock(_socket_mutex);
                _socket_server->send(bad_msg.dump() + '\n');
                return;
            }

            _execution_pool->enqueue(message);
        });


    auto on_err = [this](const std::string& err_msg) {
        json err = {
            { "status", "ERROR" },
            { "message", err_msg }
        };
        std::lock_guard<std::mutex> lock(_socket_mutex);
        _socket_server->send(err.dump() + '\n');
    };

    _socket_server->on_err(on_err);

    _execution_pool->on_err(on_err);
}


bool ExecutionServer::validate_json_msg(const json& msg)
{
    if (!msg.contains("player_id") || !msg["player_id"].is_number())         return false; 
    if (!msg.contains("game_id") || !msg["game_id"].is_number())             return false; 
    if (!msg.contains("user_code") || !msg["user_code"].is_string())         return false; 
    if (!msg.contains("test_code") || !msg["test_code"].is_string())         return false; 
    if (!msg.contains("inputs_code") || !msg["inputs_code"].is_array())      return false; 
    if (msg["inputs_code"].size() > 0 && !msg["inputs_code"][0].is_string()) return false;

    return true; 
}
