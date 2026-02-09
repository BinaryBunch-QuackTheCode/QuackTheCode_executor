

#pragma once 


#include "execution_pool.hpp"
#include <nlohmann/json.hpp> 
#include <hiredis/hiredis.h> 


class ExecutionRedisPool : public ExecutionPool
{
public: 
    ExecutionRedisPool(const std::string& ip_addr, uint16_t port);
    ~ExecutionRedisPool() { if (_redis_ctx) redisFree(_redis_ctx); }
    void enqueue(const nlohmann::json& message) override;


private: 
    redisContext* _redis_ctx = nullptr; 
};

