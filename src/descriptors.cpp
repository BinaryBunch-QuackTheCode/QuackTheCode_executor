

#include "descriptors.hpp"
#include <fcntl.h> 
#include <system_error> 

Pipe create_pipe()
{
    int pipe[2];
    if (pipe2(pipe, O_CLOEXEC) != 0)
    {
        throw std::system_error(errno, std::generic_category(), "pipe failed");
    }
    return { .read_end = UniqueFD(pipe[0]), .write_end = UniqueFD(pipe[1]) };
}
