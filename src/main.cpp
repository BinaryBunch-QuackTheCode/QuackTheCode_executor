


#include "execution_server.hpp"
#include <iostream> 

int main(int argc, char** argv)
{
    Config config; 
    std::string config_file_path = argc > 1 ? argv[1] : "config/config.json";
    try
    {
        config = parse_config(config_file_path);
    }
    catch (const std::exception& err)
    {
        std::cerr << "Could not configure from config file " << config_file_path << ": " << err.what() << std::endl;
        return 1; 
    }

    std::cout << "Starting Execution Server via a ";

    if (config.socket_type == SocketType::UNIX)
    {
        std::cout << "Unix socket. Using path: " << config.unix_args->path << std::endl; 
    }
    else if (config.socket_type == SocketType::TCP)
    {
        std::cout << "TCP socket. Using IP addr: "  << config.tcp_args->ip_addr << " and port: " << config.tcp_args->port << std::endl;
    }

    ExecutionServer(config).run(); 

    return 0; 
}
