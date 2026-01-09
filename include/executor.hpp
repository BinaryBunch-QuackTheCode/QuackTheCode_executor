

#pragma once
#include <string> 

struct ExecutionOutput
{
    std::string stdout; 
    std::string stderr; 
};

class Executor
{
public: 

    ExecutionOutput execute(const std::string& code); 


private: 
    void set_nonblocking(int fd);
    int epfd_;

};
