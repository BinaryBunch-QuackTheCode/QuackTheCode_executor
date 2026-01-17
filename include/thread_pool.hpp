
#pragma once
#include "executor.hpp"
#include <condition_variable>
#include <functional>
#include <nlohmann/json.hpp>
#include <queue>
#include <thread>
#include <vector>

/// @breif Thread pool of workers waiting for code to execute 
class ExecutionThreadPool
{
  public:
    struct Task
    {
        std::function<std::vector<ExecutionResult>(const nlohmann::json&)> task_func;
        nlohmann::json task_msg;
    };
    /// @breif Create a thread pool 
    /// @param num_threads Specify the number of threads used in the pool 
    ExecutionThreadPool(size_t num_threads = std::thread::hardware_concurrency());

    ~ExecutionThreadPool();

    /// @breif Set a callback to run when an execution of code is complete. 
    /// @param func Function that expects the message that started the execution and the output of the execution 
    void on_execution_complete(std::function<void(nlohmann::json message, std::vector<ExecutionResult>)> func)
    {
        _on_execution_complete_func = std::move(func);
    };

    /// @breif Set a callback to run if an execution errors 
    /// @param func Function the to run given an error message
    void on_err(std::function<void(const std::string&)> func) { _on_err_func = std::move(func); }

    /// @breif Add an execution task to the pool 
    /// @param Task Includes the function that should produce an ExecutionOutput given a JSON message input
    ///             alongside the corresponding message received 
    void enqueue(Task task);

  private:

    std::function<void(nlohmann::json message, std::vector<ExecutionResult>)> _on_execution_complete_func;
    std::function<void(const std::string&)> _on_err_func; 

    std::vector<std::thread> _workers;
    std::mutex               _queue_mutex;
    std::condition_variable  _cv;
    std::queue<Task>         _tasks;
    bool                     _stop = false;
};
