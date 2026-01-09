


#include "executor.hpp"
#include <iostream> 

int main()
{
    Executor executor; 

    auto output = executor.execute("prnt('Hello world')");

    std::cout << "Std Out: \n" << output.stdout << std::endl;  
    std::cout << "Std Err: \n" << output.stderr << std::endl; 

    return 0; 
}
