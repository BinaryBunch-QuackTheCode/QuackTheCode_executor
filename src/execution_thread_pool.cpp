
#include "execution_thread_pool.hpp"
#include <mutex>

using json = nlohmann::json;

ExecutionThreadPool::ExecutionThreadPool(const Executor& exe, size_t num_threads) : _executor(exe)
{
    for (size_t i = 0; i < num_threads; ++i)
    {
        _workers.emplace_back(
            [this]
            {
                while (true)
                {
                    json message; 
                    {
                        std::unique_lock<std::mutex> lock(_queue_mutex);

                        _cv.wait(lock, [this] { return !_tasks.empty() || _stop; });

                        if (_stop && _tasks.empty())
                        {
                            return;
                        }

                        message = std::move(_tasks.front());
                        _tasks.pop();
                    }
                    try
                    {
                        auto result = _executor.execute(_json_to_job_func(message));

                        _on_execution_complete_func(std::move(message), result);
                    }
                    catch (const std::exception& err)
                    {
                        if (_on_err_func != nullptr)
                            _on_err_func(err.what());
                    }
                }
            });
    }
}

ExecutionThreadPool::~ExecutionThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(_queue_mutex);
        _stop = true;
    }

    _cv.notify_all();

    for (auto& thread : _workers)
    {
        thread.join();
    }
}

void ExecutionThreadPool::enqueue(const json& message)
{
    {
        std::unique_lock<std::mutex> lock(_queue_mutex);
        _tasks.push(message);
    }
    _cv.notify_one();
}

