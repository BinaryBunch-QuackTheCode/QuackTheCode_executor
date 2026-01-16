
#include "execution_server.hpp"
#include "executor.hpp"
#include "unix_socket_server.hpp"

#include <nlohmann/json.hpp>

#include <iostream> 
#include <arpa/inet.h>
#include <sys/socket.h>

using json = nlohmann::json;

ExecutionServer::ExecutionServer(const Args& args) : _executor(args.jail_config_path)
{
    switch (args.socket_type)
    {
    case ExecutionServer::SocketType::UNIX:
        _socket_server = std::make_unique<UnixSocketServer>(args.socket_path);
        break;
    default:
        throw std::invalid_argument("Unsupported socket type");
    }

    _execution_pool.on_execution_complete(
        [this](json message, ExecutionOutput output) 
        {
            json output_json = {
                {"game_id",   message["game_id"]},  
                {"player_id", message["player_id"]}, 
                {"stdout",    std::move(output.stdout)}, 
                {"stderr",    std::move(output.stderr)}};

            _socket_server->send(output_json.dump() + '\n');
        });

    _socket_server->on_read(
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
