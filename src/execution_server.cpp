
#include "execution_server.hpp"
#include "execution_emulated_pool.hpp"
#include "execution_redis_pool.hpp" 
#include "execution_thread_pool.hpp"
#include "execmsg.hpp"
#include "tcp_socket_server.hpp"
#include "unix_socket_server.hpp"
#include <iostream> 

#include <nlohmann/json.hpp>

#include <arpa/inet.h>
#include <sys/socket.h>

using json = nlohmann::json;

ExecutionServer::ExecutionServer(const Config& config) : _config(config)
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

    switch (config.execution_pool_type)
    {
    case ExecutionPoolType::THREAD_POOL:
        _execution_pool = std::make_unique<ExecutionThreadPool>(config.sandbox_cfg_path, config.num_threads);
        break;
    case ExecutionPoolType::EMULATED:
        _execution_pool = std::make_unique<ExecutionEmulatedPool>();
        break;
    case ExecutionPoolType::REDIS: 
        _execution_pool = std::make_unique<ExecutionRedisPool>("localhost", 6379);
        break;
    default:
        throw std::invalid_argument("Unsupported execution pool type");
    }

    _execution_pool->on_execution_complete(
        [this](const json& result)
        {
            std::lock_guard<std::mutex> lock(_socket_mutex);
            _socket_server->send(result.dump() + '\n');
        });

    _socket_server->on_recv(
        [this](json message)
        {
#ifdef DEBUG_BUILD
            std::cout << "Received message: " << message.dump(4) << std::endl;
#endif
            if (!execmsg::validate_message(message))
            {
                std::lock_guard<std::mutex> lock(_socket_mutex);
                _socket_server->send(execmsg::create_error_message("Invalid parameters in JSON message").dump() + '\n');
                return;
            }

            _execution_pool->enqueue(message);
        });

    _socket_server->on_err(
        [this](const std::string& err_msg)
        {
            std::lock_guard<std::mutex> lock(_socket_mutex);
            _socket_server->send(execmsg::create_error_message(err_msg).dump() + '\n');
        }
    );

}

