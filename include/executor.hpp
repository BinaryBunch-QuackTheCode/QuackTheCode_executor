

#pragma once
#include <atomic>
#include <string>

struct ExecutionOutput
{
    std::string stdout;
    std::string stderr;
};

class Executor
{
  public:
    Executor(const std::string& config_path) : _config_path(config_path) {}

    ExecutionOutput execute(const std::string& user_code, const std::string& test_code);

  private:
    void run_jail(const std::string& jail_dir, int stdin_read, int stdin_write, int stdout_read, 
                  int stdout_write, int stderr_read, int stderr_write);

    unsigned int    generate_execution_id();
    void            set_nonblocking(int fd);
    ExecutionOutput epoll_fds(const std::string& code, int stdin_write, int stdout_read, int stderr_read);
    bool            read_fd(std::string* output, char* read_buf, size_t read_buf_size, int fd);
    int             epfd_;

    const std::string _config_path; 

    inline static std::atomic<unsigned int> _jails{0};
};
