


#include "execution_server.hpp"

int main()
{

    ExecutionServer({
        .socket_type = ExecutionServer::SocketType::UNIX,
        .socket_path = "/tmp/executor.sock",
        .jail_config_path = "../config/sandbox.cfg"
    }).run(); 

    return 0; 
}
