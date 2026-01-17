

#pragma once

#include <functional>
#include <nlohmann/json.hpp>

///@breif Byte Streamed and Connection oriented socket server.
///       Derived classes must implement how the socket fd is created and if
///       more desconstruction needs to be done beyond closing the socket fd.
///       Socket server handles message parsing and function callabcks
///
///       The socket server is meant to only handle one client at a time.
///       The server will be free to accept a client again if the connection is closed.
class SocketServer
{

  public:
    /// @breif Set a callback to run every time a JSON message is received
    /// @param func Callback function working on that json
    void on_recv(std::function<void(const nlohmann::json&)> func) { _on_read_func = std::move(func); };

    /// @breif Set a callback to run anytime an error occurs in parsing 
    /// @param func Callback function on err
    void on_err(std::function<void(const std::string&)> func) { _on_err_func = std::move(func); }

    /// @breif Send a message to the client
    /// @param msg Should be JSON with a newline delimiter
    void send(const std::string& msg);

    /// @breif Begins listenitng on the socket waiting to accept a connection. Will
    ///        then begin execution handeling once a client connects.
    void run();

    virtual ~SocketServer() { close(_socket_fd); };

  private:
    virtual int create_socket_fd() = 0;

    /// @breif Handles execution loop during the entire lifetime of the client connection
    void handle_client(int client_fd);

    std::function<void(const nlohmann::json&)> _on_read_func = nullptr;
    std::function<void(const std::string&)>    _on_err_func  = nullptr;

    int _socket_fd;
    int _client_fd = -1;
};
