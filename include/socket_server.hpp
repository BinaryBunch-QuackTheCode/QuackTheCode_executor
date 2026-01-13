

#pragma once

#include <functional>
#include <nlohmann/json.hpp>

class SocketServer
{

  public:
    void on_read(const std::function<void(const nlohmann::json&)>& func) { _on_read_func = func; };
    void send(const std::string& msg);
    void run();

    virtual ~SocketServer() { close(_socket_fd); };

  private:
    virtual int create_socket_fd() = 0;

    void                                       handle_client(int clinet_fd);
    std::function<void(const nlohmann::json&)> _on_read_func = nullptr;

    int _socket_fd;
};
