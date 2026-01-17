
#include "thread_pool.hpp"
#include <mutex>

ExecutionThreadPool::ExecutionThreadPool(size_t num_threads)
{

    for (size_t i = 0; i < num_threads; ++i)
    {
        _workers.emplace_back(
            [this]
            {
                while (true)
                {
                    Task task; 
                    {
                        std::unique_lock<std::mutex> lock(_queue_mutex);

                        _cv.wait(lock, [this] { return !_tasks.empty() || _stop; });

                        if (_stop && _tasks.empty())
                        {
                            return;
                        }

                        task = std::move(_tasks.front());
                        _tasks.pop();
                    }
                    try
                    {
                        _on_execution_complete_func(std::move(task.task_msg), task.task_func(task.task_msg));
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

void ExecutionThreadPool::enqueue(Task task)
{
    {
        std::unique_lock<std::mutex> lock(_queue_mutex);
        _tasks.emplace(std::move(task));
    }
    _cv.notify_one();
}

