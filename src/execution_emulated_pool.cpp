
#include "execution_emulated_pool.hpp"

void ExecutionEmulatedPool::enqueue(const nlohmann::json& message)
{
    auto job = _json_to_job_func(message);
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
    _on_execution_complete_func(message, results);
}


