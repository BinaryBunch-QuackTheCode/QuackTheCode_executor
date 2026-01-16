
#include "executor.hpp"
#include <fcntl.h>
#include <filesystem>
#include <string>
#include <sys/epoll.h>
#include <sys/wait.h>
#include <system_error>
#include <unistd.h>

ExecutionOutput Executor::execute(const std::string& user_code, const std::string& test_code)
{
    std::string full_code = user_code + '\n' + test_code;

    int in[2], out[2], err[2];
    if (pipe2(in, O_CLOEXEC) != 0 || pipe2(out, O_CLOEXEC) != 0 || pipe2(err, O_CLOEXEC) != 0)
    {
        throw std::system_error(errno, std::generic_category(), "Executor Pipe failed");
    }

    int stdin_read   = in[0];
    int stdin_write  = in[1];
    int stdout_read  = out[0];
    int stdout_write = out[1];
    int stderr_read  = err[0];
    int stderr_write = err[1];

    std::string jail_dir = "/tmp/jail_" + std::to_string(generate_execution_id());
    std::filesystem::create_directory(jail_dir);

    pid_t pid = fork();

    if (pid == -1)
    {
        throw std::system_error(errno, std::generic_category(), "fork failed");
    }

    if (pid == 0) // child
    {
        run_jail(jail_dir, stdin_read, stdin_write, stdout_read, stdout_write, stderr_read, stderr_write);
        _exit(1);
    }

    // parent
    close(stdin_read);
    close(stdout_write);
    close(stderr_write);

    auto output = epoll_fds(full_code, stdin_write, stdout_read, stderr_read);

    int status;
    waitpid(pid, &status, 0);

    std::filesystem::remove_all(jail_dir);

    return output;
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

void Executor::run_jail(const std::string& jail_dir, int stdin_read, int stdin_write, int stdout_read, int stdout_write, int stderr_read,
                        int stderr_write)
{
    dup2(stdin_read, STDIN_FILENO);
    dup2(stdout_write, STDOUT_FILENO);
    dup2(stderr_write, STDERR_FILENO);

    close(stdin_write);
    close(stdout_read);
    close(stderr_read);

    execlp("nsjail", "nsjail", "--quiet", "--config", _config_path.c_str(), "--bindmount", (jail_dir + ":/sandbox").c_str(), "--",
           "/usr/local/bin/python3", "-", (char*)nullptr);
}

ExecutionOutput Executor::epoll_fds(const std::string& code, int stdin_write, int stdout_read, int stderr_read)
{
    set_nonblocking(stdin_write);
    set_nonblocking(stdout_read);
    set_nonblocking(stderr_read);

    int ep = epoll_create1(0);

    epoll_event ev;
    ev.events  = EPOLLOUT;
    ev.data.fd = stdin_write;
    epoll_ctl(ep, EPOLL_CTL_ADD, stdin_write, &ev);

    ev.events  = EPOLLIN;
    ev.data.fd = stdout_read;
    epoll_ctl(ep, EPOLL_CTL_ADD, stdout_read, &ev);

    ev.events  = EPOLLIN;
    ev.data.fd = stderr_read;
    epoll_ctl(ep, EPOLL_CTL_ADD, stderr_read, &ev);

    size_t written    = 0;
    bool   stdin_done = false, stdout_open = true, stderr_open = true;

    std::string stdout_buf, stderr_buf;
    // tmp buffer for reading outputs, will be appended to the above buffers
    char tmp_buf[4096];

    epoll_event events[3];

    try
    {
        while (!stdin_done || stdout_open || stderr_open)
        {
            int n = epoll_wait(ep, events, 3, -1);

            for (int i = 0; i < n; i++)
            {
                int fd = events[i].data.fd;

                if (fd == stdin_write && !stdin_done)
                {
                    ssize_t w = write(stdin_write, code.data() + written, code.size() - written);
                    if (w > 0)
                    {
                        written += w;
                        if (written == code.size())
                        {
                            close(stdin_write);
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
    }
    catch (const std::system_error& err)
    {
        close(stdout_read);
        close(stderr_read);
        if (!stdin_done) 
            close(stdin_write);
        close(ep);     
        throw err; 
    }

    close(stdout_read);
    close(stderr_read);
    if (!stdin_done) 
        close(stdin_write);
    close(ep);     

    return {.stdout = std::move(stdout_buf), .stderr = std::move(stderr_buf)};
}

void Executor::set_nonblocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

unsigned int Executor::generate_execution_id() { return _jails++; }



