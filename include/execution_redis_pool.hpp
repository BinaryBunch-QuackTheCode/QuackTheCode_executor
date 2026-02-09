

#pragma once

#include "execution_pool.hpp"
#include <hiredis/hiredis.h>
#include <nlohmann/json.hpp>
#include <thread>

class ExecutionRedisPool : public ExecutionPool
{
  public:
    ExecutionRedisPool(const std::string& ip_addr, uint16_t port);
    ~ExecutionRedisPool();
    void enqueue(const nlohmann::json& message) override;

  private:
    redisContext* _redis_ctx = nullptr;
    std::thread   _results_thread;
};
