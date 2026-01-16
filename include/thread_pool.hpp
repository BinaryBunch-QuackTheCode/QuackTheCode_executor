
#pragma once
#include "executor.hpp"
#include <condition_variable>
#include <functional>
#include <nlohmann/json.hpp>
#include <queue>
#include <thread>
#include <vector>

class ExecutionThreadPool
{
  public:
    struct Task
    {
        std::function<ExecutionOutput(const nlohmann::json&)> task_func;
        nlohmann::json                                        task_msg;
    };
    ExecutionThreadPool(size_t num_threads = std::thread::hardware_concurrency());
    ~ExecutionThreadPool();
    void on_execution_complete(std::function<void(const nlohmann::json& message, const ExecutionOutput&)> func)
    {
        _on_execution_complete_func = std::move(func);
    };
    void enqueue(Task task);

  private:

    std::function<void(const nlohmann::json& message, const ExecutionOutput&)> _on_execution_complete_func;

    std::vector<std::thread> _workers;
    std::mutex               _queue_mutex;
    std::condition_variable  _cv;
    std::queue<Task>         _tasks;
    bool                     _stop = false;
};
