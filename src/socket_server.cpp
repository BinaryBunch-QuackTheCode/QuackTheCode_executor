

#include "socket_server.hpp"
#include <sys/socket.h>
#include <unistd.h>
#include <iostream> 

using json = nlohmann::json;

void SocketServer::run()
{
    if (_on_read_func == nullptr)
    {
        throw std::runtime_error("On read function not set");
    }

    _socket_fd = create_socket_fd();

    int client_fd = accept(_socket_fd, nullptr, nullptr);

    if (client_fd < 0)
    {
        throw std::system_error(errno, std::generic_category(), "accept failed");
    }

    std::cout << "accepted" << std::endl;

    handle_client(client_fd);

    close(client_fd);
}

void SocketServer::handle_client(int client_fd)
{
    constexpr size_t buffer_chunk_size = 4096;
    auto             buffer            = std::make_unique<char[]>(buffer_chunk_size);
    size_t           buffer_space      = buffer_chunk_size;
    size_t           buffer_size       = 0;
    size_t           to_search         = 0;

    while (true)
    {
        ssize_t num_bytes = recv(client_fd, buffer.get() + buffer_size, buffer_space - buffer_size, 0);
        buffer_size += num_bytes;

        if (num_bytes == 0)
        {
            return;
        }

        if (num_bytes < 0)
        {
            throw std::system_error(errno, std::generic_category(), "recv failed");
        }

        std::optional<size_t> msg_delim_idx;
        for (size_t i = to_search; i < buffer_size; i++)
        {
            if (buffer[i] == '\n')
            {
                msg_delim_idx = i;
            }
        }

        if (!msg_delim_idx)
        {
            to_search = buffer_size;

            if (buffer_size == buffer_space)
            {
                // Resize
                buffer_space += buffer_chunk_size;
                auto tmp = std::make_unique<char[]>(buffer_space);
                memcpy(tmp.get(), buffer.get(), buffer_size);
                buffer = std::move(tmp);
            }
            continue;
        }

        buffer[*msg_delim_idx] = '\0';

        _on_read_func(json::parse(buffer.get()));

        to_search   = 0;
        buffer_size = 0;
    }
}

void SocketServer::send(const std::string& msg)
{
    ssize_t total_sent = 0; 
    while (total_sent < msg.size())
    {
        ssize_t sent = ::send(_socket_fd, msg.data() + total_sent, msg.size() - sent, 0);
        if (sent > 0)
        {
            total_sent += sent; 
        } 
        else if (sent == 0)
        {
            return;
        }
        else
        {
            throw std::system_error(errno, std::generic_category(), "send failed");
        }
    }
}




