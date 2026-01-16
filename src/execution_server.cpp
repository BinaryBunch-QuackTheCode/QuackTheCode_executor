
#include "execution_server.hpp"
#include "executor.hpp"
#include "unix_socket_server.hpp"
#include "tcp_socket_server.hpp"

#include <nlohmann/json.hpp>

#include <arpa/inet.h>
#include <sys/socket.h>

using json = nlohmann::json;

ExecutionServer::ExecutionServer(const Config& config) : _config(config), _executor(config.sandbox_cfg_path), _execution_pool(config.num_threads)
{
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

    _execution_pool.on_execution_complete(
        [this](json message, ExecutionOutput output) 
        {
            json output_json = {
                {"game_id",     message["game_id"]},  
                {"player_id",   message["player_id"]}, 
                {"stdout",      std::move(output.stdout)}, 
                {"stderr",      std::move(output.stderr)}, 
            };

            _socket_server->send(output_json.dump() + '\n');
        });

    _socket_server->on_recv(
        [this](json message)
        {
            _execution_pool.enqueue({
                .task_func = [this](const json& message) 
                { 
                    std::string user_code = message["user_code"];
                    std::string test_code = message["test_code"];
                    return _executor.execute(user_code, test_code); 
                }, 
                .task_msg = std::move(message)
            });
        });
}
