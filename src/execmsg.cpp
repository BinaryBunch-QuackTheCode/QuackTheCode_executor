
#include "execmsg.hpp"

using json = nlohmann::json;

namespace execmsg
{

ExecutionJob message_to_job(const json& job)
{
    return ExecutionJob{
        .user_code   = job["user_code"], 
        .inputs_code = job["inputs_code"], 
        .test_code   = job["test_code"]
    }; 
}

json results_to_response(const json& message, const std::vector<ExecutionResult>& results)
{
    json results_json = json::array();
    for (const auto& result : results)
    {
        results_json.push_back({
            {"cpu_time_ms", result.cpu_time_ms},
            {"stdout", std::move(result.stdout)},
            {"stderr", std::move(result.stderr)},
            {"succeeded", result.succeeded},
            {"time_limit_exceeded", result.time_limit_exceeded},
            {"tests_failed", result.tests_failed},
            {"unknown_error", result.unknown_error},
        });
    };

    json response = {
        {"status",    "OK"}, 
        {"game_id",   message["game_id"]}, 
        {"player_id", message["player_id"]}, 
        {"results",   std::move(results_json)}
    };

    return response; 
}

bool validate_message(const json& msg)
{
    if (!msg.contains("player_id"))
        return false;
    if (!msg.contains("game_id"))
        return false;
    if (!msg.contains("user_code") || !msg["user_code"].is_string())
        return false;
    if (!msg.contains("test_code") || !msg["test_code"].is_string())
        return false;
    if (!msg.contains("inputs_code") || !msg["inputs_code"].is_array())
        return false;
    if (msg["inputs_code"].size() > 0 && !msg["inputs_code"][0].is_string())
        return false;

    return true;
}


nlohmann::json create_error_message(const std::string& err)
{
    return {
        { "status", "ERROR" }, 
        { "message", err    }
    };
}


};

