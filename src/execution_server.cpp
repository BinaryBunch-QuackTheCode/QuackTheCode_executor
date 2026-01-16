
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
        [this](json message, std::vector<std::pair<ExecutionOutput, ExecutionStats>> outputs) 
        {
            std::vector<json> results; 
            for (const auto [output, stats] : outputs)
            {
                json output_json = {
                    {"stdout", std::move(output.stdout)}, 
                    {"stderr", std::move(output.stderr)}, 
                };

                json stats_json = { 
                    {"cpu_time_ms", stats.cpu_time_ms},
                    {"succeeded",   stats.succeeded},
                    {"reason",      stats.reason}
                };
                results.push_back({
                    {"output", std::move(output_json)},
                    {"stats",  std::move(stats_json)}
                });
            }
            json response = {
                {"game_id",   message["game_id"]},  
                {"player_id", message["player_id"]}, 
                {"results",   std::move(results)}
            };

            std::lock_guard<std::mutex> lock(_socket_mutex);
            _socket_server->send(response.dump() + '\n');
        });

    _socket_server->on_recv(
        [this](json message)
        {
            _execution_pool.enqueue({
                .task_func = [this](const json& message) 
                { 
                    if (!_config.is_emulated)
                    {
                        std::string user_code           = message["user_code"];
                        std::vector<std::string> inputs = message["inputs_code"];
                        std::string test_code            = message["test_code"];
                        return _executor.execute(user_code, inputs, test_code); 
                    }
                    std::vector<std::pair<ExecutionOutput, ExecutionStats>> results; 
                    for (const auto& _ : message["inputs_code"])
                    {
                        results.push_back(std::pair<ExecutionOutput, ExecutionStats> {
                            ExecutionOutput{
                                .stdout      = "Emulated stdout\n",
                                .stderr      = "Emulated stderr\n", 
                            }, 
                            ExecutionStats{
                                .cpu_time_ms = 10,
                                .succeeded = true, 
                                .reason = ""
                            }
                        });
                    }
                    return results; 
                }, 
                .task_msg = std::move(message)
            });
        });
}
