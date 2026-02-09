

#include "execution_redis_pool.hpp"
#include <iostream> 

using json = nlohmann::json;

ExecutionRedisPool::~ExecutionRedisPool()
{
    if (_redis_ctx)
        redisFree(_redis_ctx);
    _results_thread.join();
}

ExecutionRedisPool::ExecutionRedisPool(const std::string& ip_address, uint16_t port)
{
    _redis_ctx = redisConnect(ip_address.c_str(), port);
    if (_redis_ctx->err)
    {
        throw std::runtime_error("Could not connect to Redis server on IP address " 
                                 + ip_address + " on port " + std::to_string(port)
                                 + ": " + _redis_ctx->errstr);
    }

    _results_thread = std::thread([this, ip_address, port]{
        redisContext* recv_ctx = redisConnect(ip_address.c_str(), port);
        if (recv_ctx->err)
        {
            throw std::runtime_error("Could not connect to Redis server on IP address " 
                                     + ip_address + " on port " + std::to_string(port)
                                     + ": " + recv_ctx->errstr);
        }

        while (true)
        {
            redisReply* reply = (redisReply*)redisCommand(recv_ctx, "BLPOP exec_result 0");  
            if (reply->type == REDIS_REPLY_ARRAY && reply->elements == 2)
            { 
                json response = json::parse(reply->element[1]->str);  
                _on_execution_complete_func(response);
            }
            freeReplyObject(reply);
        }
        redisFree(recv_ctx);
    });
}

void ExecutionRedisPool::enqueue(const nlohmann::json& message)
{
    redisReply* reply = (redisReply*)redisCommand(_redis_ctx, "RPUSH exec %s", message.dump().c_str());

#ifdef DEBUG_BUILD
    if (reply->type != REDIS_REPLY_ERROR)
    {
        std::cout << "Message sent to REDIS exec queue" << std::endl;
    }
    else
    {
        std::cout << "REDIS Error: " << reply->str << std::endl;
    }
#endif
    freeReplyObject(reply);
}


