

#pragma once

#include "executor.hpp"
#include "nlohmann/json.hpp"

namespace execmsg
{

ExecutionJob   message_to_job(const nlohmann::json& job);
nlohmann::json results_to_response(const nlohmann::json& message, const std::vector<ExecutionResult>& results);
bool           validate_message(const nlohmann::json& msg);
nlohmann::json create_error_message(const std::string& err);

};
