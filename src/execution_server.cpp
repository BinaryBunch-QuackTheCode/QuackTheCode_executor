
#include "execution_server.hpp"
#include "executor.hpp"
#include "unix_socket_server.hpp"

#include <nlohmann/json.hpp>

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

    _socket_server->on_read(
        [this](const json& message)
        {
            int         player_id = message["player_id"];
            std::string user_code = message["user_code"];
            std::string test_code = message["test_code"];

            auto output = _executor.execute(user_code, test_code);

            json output_json = {
                {"player_id", player_id},
                {"stdout", std::move(output.stdout)}, 
                {"stderr", std::move(output.stderr)}
            }; 

            _socket_server->send(output_json.dump());
        });
}
