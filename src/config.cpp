

#include "config.hpp"
#include <fstream> 
#include <thread> 
#include <nlohmann/json.hpp> 


using json = nlohmann::json;

Config parse_config(const std::string& config_file_path)
{
    Config output; 
    std::ifstream config_file;
    config_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    config_file.open(config_file_path);

    json config = json::parse(config_file);

    if (!config.contains("socket"))
    {
        throw std::invalid_argument("socket config not found");
    }

    json& socket_config = config["socket"];

    if (!socket_config.contains("socket_type"))
    {
        throw std::invalid_argument("socket_type not found in config file");
    }

    if (socket_config["socket_type"] == "unix")
    {
        output.socket_type = SocketType::UNIX; 
        if (!socket_config.contains("path"))
        {
            throw std::invalid_argument("unix socket missing path");
        }
        UnixSocketArgs args; 
        output.unix_args = args; 
        output.unix_args->path = socket_config["path"];
    }
    else if (socket_config["socket_type"] == "tcp")
    {
        output.socket_type = SocketType::TCP; 
        if (!socket_config.contains("ip_addr") || !socket_config.contains("port"))
        {
            throw std::invalid_argument("tcp socket missing ip address or port");
        }
        TCPSocketArgs args; 
        output.tcp_args = args; 
        output.tcp_args->ip_addr = socket_config["ip_addr"];
        output.tcp_args->port = socket_config["port"];
    }
    else
    {
        throw std::invalid_argument("invalid socket type");
    }

    if (!config.contains("sandbox_config_path"))
    {
        throw std::invalid_argument("missing sandbox config path");
    }
    output.sandbox_cfg_path = config["sandbox_config_path"];

    if (!config.contains("num_threads") || config["num_threads"].empty())
    {
        output.num_threads = std::thread::hardware_concurrency();
    }
    else
    {
        output.num_threads = config["num_threads"];
    }

    if (config.contains("is_emulated"))
    {
        output.is_emulated = config["is_emulated"];
    }

    return output; 
}
