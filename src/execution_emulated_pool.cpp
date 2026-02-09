
#include "execution_emulated_pool.hpp"
#include "executor.hpp"
#include "execmsg.hpp"

void ExecutionEmulatedPool::enqueue(const nlohmann::json& message)
{
    auto job = execmsg::message_to_job(message);
    std::vector<ExecutionResult> results; 
    for (size_t _ = 0; _ < job.inputs_code.size(); _++)
    {
        results.push_back(ExecutionResult{
            .cpu_time_ms = 10,
            .succeeded   = true, 
            .stdout      = "Emulated stdout\n",
            .stderr      = "Emulated stderr\n", 
        });
    }
    _on_execution_complete_func(execmsg::results_to_response(message, results));
}


