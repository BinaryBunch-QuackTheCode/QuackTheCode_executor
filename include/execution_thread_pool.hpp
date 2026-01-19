
#pragma once
#include <condition_variable>
#include <nlohmann/json.hpp>
#include "execution_pool.hpp"
#include "executor.hpp"
#include <queue>
#include <thread>
#include <vector>

/// @breif Thread pool of workers waiting for code to execute
class ExecutionThreadPool : public ExecutionPool
{
  public:
    /// @breif Create a thread pool
    /// @param num_threads Specify the number of threads used in the pool
    ExecutionThreadPool(const Executor& exes, size_t num_threads = std::thread::hardware_concurrency());

    ~ExecutionThreadPool();

    /// @breif Add an execution task to the pool
    /// @param message Message to be executed
    void enqueue(const nlohmann::json& message) override;

  private:
    Executor _executor; 
    std::vector<std::thread>   _workers;
    std::mutex                 _queue_mutex;
    std::condition_variable    _cv;
    std::queue<nlohmann::json> _tasks;
    bool                       _stop = false;
};
