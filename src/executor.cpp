
#include "executor.hpp"
#include "descriptors.hpp"
#include <nlohmann/json.hpp> 
#include <fcntl.h>
#include <sys/resource.h> 
#include <filesystem>
#include <string>
#include <sys/epoll.h>
#include <sys/wait.h>
#include <system_error>
#include <unistd.h>

using json = nlohmann::json; 

std::vector<std::pair<ExecutionOutput, ExecutionStats>> Executor::execute(const std::string& user_code, 
                                                                          const std::vector<std::string>& test_cases_code, 
                                                                          const std::string& test_code)
{
    std::vector<std::pair<ExecutionOutput, ExecutionStats>> results; 
    results.reserve(test_cases_code.size()); 

    for (const auto& input_code : test_cases_code)
    {
        std::string full_code = user_code + '\n' + input_code + '\n' + test_code;

        // The plan is to pipe in, out and err so the parent process can
        // write to stdin and read from stdout and stderr as the child executes the isolated python code

        Pipe stdin_pipe  = create_pipe();
        Pipe stdout_pipe = create_pipe();
        Pipe stderr_pipe = create_pipe();

        std::string jail_dir = "/tmp/jail_" + std::to_string(generate_execution_id());
        std::filesystem::create_directory(jail_dir);

        pid_t pid = fork();

        if (pid == -1)
        {
            throw std::system_error(errno, std::generic_category(), "fork failed");
        }

        if (pid == 0) // child
        {
            run_jail(jail_dir, stdin_pipe, stdout_pipe, stderr_pipe);
            _exit(1);
        }

        stdin_pipe.read_end.release(); 
        stdout_pipe.write_end.release(); 
        stderr_pipe.write_end.release(); 

        // parent
        auto output = epoll_fds(full_code, std::move(stdin_pipe.write_end), stdout_pipe.read_end.get(), stderr_pipe.read_end.get());

        int status;
        rusage ru; 
        wait4(pid, &status, 0, &ru);

        // Check cpu time 
        uint32_t cpu_time_ms =
                    ru.ru_utime.tv_sec * 1000 +
                    ru.ru_utime.tv_usec / 1000;

        
        ExecutionStats stats = { .cpu_time_ms = cpu_time_ms }; 

        // Check results 
        if (WIFEXITED(status))
        {
            int exit_code = WEXITSTATUS(status);
            if (exit_code != 0)
            {
                stats.succeeded = false; 
                stats.reason = "Test Cases Failed";
            }
            else
            {
                stats.succeeded = true; 
            }
        }
        else if (WIFSIGNALED(status))
        {
            stats.succeeded = false; 
            int signal = WTERMSIG(status);
            stats.reason = signal == SIGKILL ? "Time Limit Exceeded" : "Failure Unknown";
        }

        std::filesystem::remove_all(jail_dir);

        results.push_back({ std::move(output), std::move(stats) });
    }
    return results; 
}

bool Executor::read_fd(std::string* output, char* read_buf, size_t read_buf_size, int fd)
{
    while (true)
    {
        ssize_t r = read(fd, read_buf, read_buf_size);
        if (r > 0)
        {
            output->append(read_buf, r);
        }
        else if (r == 0)
        {
            return false;
        }
        else if (errno == EWOULDBLOCK || errno == EAGAIN)
        {
            return true;
        }
        else
        {
            throw std::system_error(errno, std::generic_category(), "read failed");
        }
    }
}

void Executor::run_jail(const std::string& jail_dir, Pipe& stdin_pipe, Pipe& stdout_pipe, Pipe& stderr_pipe)
{
    dup2(stdin_pipe.read_end.get(), STDIN_FILENO);
    dup2(stdout_pipe.write_end.get(), STDOUT_FILENO);
    dup2(stderr_pipe.write_end.get(), STDERR_FILENO);

    // this is in child process so its fine
    close(stdin_pipe.write_end.get());
    close(stdout_pipe.read_end.get());
    close(stderr_pipe.read_end.get());

    execlp("nsjail", "nsjail", "--quiet", "--config", _config_path.c_str(), "--bindmount", (jail_dir + ":/sandbox").c_str(), "--",
           "/usr/local/bin/python3", "-", (char*)nullptr);
}

ExecutionOutput Executor::epoll_fds(const std::string& code, UniqueFD stdin_write, int stdout_read, int stderr_read)
{
    set_nonblocking(stdin_write.get());
    set_nonblocking(stdout_read);
    set_nonblocking(stderr_read);

    UniqueFD ep(epoll_create1(0));

    epoll_event ev;
    ev.events  = EPOLLOUT;
    ev.data.fd = stdin_write.get();
    epoll_ctl(ep.get(), EPOLL_CTL_ADD, stdin_write.get(), &ev);

    ev.events  = EPOLLIN;
    ev.data.fd = stdout_read;
    epoll_ctl(ep.get(), EPOLL_CTL_ADD, stdout_read, &ev);

    ev.events  = EPOLLIN;
    ev.data.fd = stderr_read;
    epoll_ctl(ep.get(), EPOLL_CTL_ADD, stderr_read, &ev);

    size_t written    = 0;
    bool   stdin_done = false, stdout_open = true, stderr_open = true;

    std::string stdout_buf, stderr_buf;

    // tmp buffer for reading outputs, will be appended to the above buffers
    char tmp_buf[4096];

    epoll_event events[3];

    // Write to stdin while reading from stdout and stderr
    while (!stdin_done || stdout_open || stderr_open)
    {
        int n = epoll_wait(ep.get(), events, 3, -1);

        for (int i = 0; i < n; i++)
        {
            int fd = events[i].data.fd;

            if (fd == stdin_write.get() && !stdin_done)
            {
                ssize_t w = write(stdin_write.get(), code.data() + written, code.size() - written);
                if (w > 0)
                {
                    written += w;
                    if (written == code.size())
                    {
                        stdin_write.release(); 
                        stdin_done = true;
                    }
                }
            }
            else if (fd == stdout_read)
            {
                stdout_open = read_fd(&stdout_buf, tmp_buf, sizeof(tmp_buf), fd);
            }
            else if (fd == stderr_read)
            {
                stderr_open = read_fd(&stderr_buf, tmp_buf, sizeof(tmp_buf), fd);
            }
        }
    }

    return {.stdout = std::move(stdout_buf), .stderr = std::move(stderr_buf)};
}

void Executor::set_nonblocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

unsigned int Executor::generate_execution_id() { return _jails++; }















