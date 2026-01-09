
#include <fcntl.h>
#include <string>
#include <sys/epoll.h>
#include <unistd.h>
#include <sys/wait.h>
#include "executor.hpp"
#include <system_error> 


ExecutionOutput Executor::execute(const std::string& code)
{
    int in[2], out[2], err[2];
    if (pipe(in) != 0 || pipe(out) != 0 || pipe(err) != 0)
    {
        throw std::system_error(errno, std::generic_category(), "Executor Pipe failed");
    }

    pid_t pid = fork();

    if (pid == 0) // child
    {
        dup2(in[0], STDIN_FILENO);
        dup2(out[1], STDOUT_FILENO);
        dup2(err[1], STDERR_FILENO);

        close(in[1]);
        close(out[0]);
        close(err[0]);

        execlp("docker", "docker", "run", "--rm", "-i", "python:3.12", "python", "-", nullptr);

        _exit(1);
    }

    // parent
    close(in[0]);
    close(out[1]);
    close(err[1]);

    set_nonblocking(in[1]);
    set_nonblocking(out[0]);
    set_nonblocking(err[0]);

    int ep = epoll_create1(0);

    epoll_event ev;
    ev.events  = EPOLLOUT;
    ev.data.fd = in[1];
    epoll_ctl(ep, EPOLL_CTL_ADD, in[1], &ev);

    ev.events  = EPOLLIN;
    ev.data.fd = out[0];
    epoll_ctl(ep, EPOLL_CTL_ADD, out[0], &ev);

    ev.events  = EPOLLIN;
    ev.data.fd = err[0];
    epoll_ctl(ep, EPOLL_CTL_ADD, err[0], &ev);

    size_t written     = 0;
    bool   stdin_done  = false;
    bool   stdout_open = true;
    bool   stderr_open = true;

    std::string stdout_buf, stderr_buf;
    char        buf[4096];

    epoll_event events[3];

    while (!stdin_done || stdout_open || stderr_open)
    {
        int n = epoll_wait(ep, events, 3, -1);

        for (int i = 0; i < n; i++)
        {
            int fd = events[i].data.fd;

            if (fd == in[1] && !stdin_done)
            {
                ssize_t w = write(in[1], code.data() + written, code.size() - written);
                if (w > 0)
                {
                    written += w;
                    if (written == code.size())
                    {
                        close(in[1]);
                        stdin_done = true;
                    }
                }
            }
            if (fd == out[0])
            {
                ssize_t r = read(out[0], buf, sizeof(buf));
                if (r > 0)
                {
                    stdout_buf.append(buf, r);
                }
                else
                {
                    close(out[0]);
                    stdout_open = false;
                }
            }

            if (fd == err[0])
            {
                ssize_t r = read(err[0], buf, sizeof(buf));
                if (r > 0)
                    stderr_buf.append(buf, r);
                else
                {
                    close(err[0]);
                    stderr_open = false;
                }
            }
        }
    }
    int status;
    waitpid(pid, &status, 0);

    return { .stdout = stdout_buf, .stderr = stderr_buf };
}




void Executor::set_nonblocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}
