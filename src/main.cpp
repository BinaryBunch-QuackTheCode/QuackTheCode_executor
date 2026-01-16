


#include "execution_server.hpp"
#include <iostream> 

/* 
 * Entry Point to the Executor Program. 
 *  
 * The Executor program will listen on a specified socket type (UNIX or TCP), expecting 
 * JSON messages delimited by the newline character. 
 * The message should be in the following form: 
 * {
 *     player_id: int,
 *     game_id: int, 
 *     user_code: str, 
 *     test_code: list[str]
 * }\n
 *
 * The Executor will then execute the program included in the JSON message in an sandboxed enviornment
 *
 * The Executor will then send JSON results back through the connected socket
 * The response message will be in the following form: 
 * {
 *     player_id: int
 *     game_id: int
 *     stdout: str
 *     stderr: str
 * }
 *
*/ 


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
