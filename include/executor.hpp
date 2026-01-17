

#pragma once
#include <atomic>
#include <descriptors.hpp>
#include <string>
#include <vector>

struct ExecutionResult
{
    long cpu_time_ms;
    bool succeeded           = false;
    bool time_limit_exceeded = false;
    bool tests_failed        = false; 
    bool unknown_error       = false;

    std::string stdout;
    std::string stderr;

    std::string err_msg; 
};

///@breif Executes code in a sandbox enviornment
class Executor
{
  public:
    /// @breif Create an executor with a nsjail confg file path
    /// @param config_path Config path to use for nsjail
    Executor(const std::string& config_path) : _config_path(config_path) {}

    /// @breif Execute python code
    /// @user_code Code submitted from the user
    /// @inputs_Code that includes the inputs to run into test_code
    /// @test_code Test code, appened after the user code
    /// @returns Output text from the execution and stats from execution
    std::vector<ExecutionResult> execute(const std::string& user_code, const std::vector<std::string>& inputs_code, const std::string& test_code);

  private:
    /// @breif Run an isolated program with all of the necessary file descriptors.
    /// @param jail_dir Directory path of the isolated env
    /// @param stdin_pipe Stdin pipe to read from parent process
    /// @param stdout_pipe Stdin pipe to output to parent process
    /// @param stderr_pipe Stdin pipe to output to parent process
    void run_jail(const std::string& jail_dir, Pipe& stdin_pipe, Pipe& stdout_pipe, Pipe& stderr_pipe);

    /// @breif Generate a unique execution ID among executions (thread safe)
    /// @return Execution ID
    unsigned int generate_execution_id();

    /// @breif Set a file descriptor to non-blocking mode
    /// @param fd Descriptor to set
    void set_nonblocking(int fd);

    /// @breif Uses epoll to write code to stdin and read from stdout and stderr concurrently
    /// @param code Code to write to stdin
    /// @param stdin_write File descriptor of stdin write end. Now owns the stdin and will close it
    /// @param stdout_read File descriptor of stdout read end
    /// @param stderr_read File descriptor of stderr read end
    /// @return Captured stdout and stderr
    std::pair<std::string, std::string> epoll_fds(const std::string& code, UniqueFD stdin_write, int stdout_read, int stderr_read);

    /// @breif Write messages to an output given a file descriptor to read from
    /// @param output String that will be appended with result of read
    /// @param read_buf Intermediate buffer to buffer the reads
    /// @param read_buf_size Size of the read_buf
    /// @param fd File descriptor to read from
    bool read_fd(std::string* output, char* read_buf, size_t read_buf_size, int fd);

    /// @breif Fill the result flags based on the status of the programs exit 
    /// @param status status code or signal of the exited program 
    /// @result will fill flags of result
    void parse_status(int status, ExecutionResult& result);

    int epfd_;

    const std::string _config_path;

    inline static std::atomic<unsigned int> _jails{0};
};
