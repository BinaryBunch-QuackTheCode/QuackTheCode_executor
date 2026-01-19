
#pragma once
#include "execution_pool.hpp"

class ExecutionEmulatedPool : public ExecutionPool
{
  public:
    void enqueue(const nlohmann::json& message) override;
};
