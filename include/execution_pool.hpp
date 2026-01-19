
#pragma once
#include <functional> 
#include <nlohmann/json.hpp>
#include "executor.hpp"

class ExecutionPool
{
public: 

    /// @breif Set a callback to convert json messages to an execution job. May not be needed by the type 
    ///        of execution pool 
    void json_to_execution_job(std::function<ExecutionJob(const nlohmann::json& message)> func) { _json_to_job_func = std::move(func); };

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
    /// @param message Message to be executed 
    virtual void enqueue(const nlohmann::json& message) = 0;

protected: 
    std::function<ExecutionJob(const nlohmann::json& message)> _json_to_job_func; 
    std::function<void(const std::string&)> _on_err_func; 
    std::function<void(nlohmann::json message, std::vector<ExecutionResult>)> _on_execution_complete_func;
};

