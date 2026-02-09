

#include "execution_redis_pool.hpp"
#include <iostream> 


ExecutionRedisPool::ExecutionRedisPool(const std::string& ip_address, uint16_t port)
{
    _redis_ctx = redisConnect(ip_address.c_str(), port);
    if (_redis_ctx->err)
    {
        throw std::runtime_error("Could not connect to Redis server on IP address " + ip_address + " on port " + std::to_string(port)
                                 + ": " + _redis_ctx->errstr);
    }
}

void ExecutionRedisPool::enqueue(const nlohmann::json& message)
{
    redisReply* reply = (redisReply*)redisCommand(_redis_ctx, "PUBLISH exec %s", message.dump().c_str());

#ifdef DEBUG_BUILD
    if (reply->type == REDIS_REPLY_INTEGER)
    {
        std::cout << "Message sent to " << reply->integer << " subscribers" << std::endl;
    }
#endif 
    freeReplyObject(reply);
}


