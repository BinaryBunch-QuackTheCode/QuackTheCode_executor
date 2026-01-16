

#pragma once
#include <unistd.h>



class UniqueFD
{

  public:
    explicit UniqueFD(int fd) : _fd(fd) {}

    void UnqiueFD(const UniqueFD& fd)  = delete;

    void operator=(const UniqueFD& fd) = delete;

    void operator=(UniqueFD&& other) 
    { 
        if (other._fd != _fd)   
        {
            _fd = other._fd; 
            other._fd = -1; 
        }
    }
    
    UniqueFD(UniqueFD&& other) 
    { 
        if (other._fd != _fd)  
        {
            _fd = other._fd; 
            other._fd = -1; 
        }
    }

    int get() { return _fd; }

    void release() { close(_fd); _fd = -1; }

    ~UniqueFD() { if (_fd != -1) close(_fd); }

  private:
    int _fd = -1;
};



struct Pipe
{
    UniqueFD read_end; 
    UniqueFD write_end; 
};

Pipe create_pipe();
