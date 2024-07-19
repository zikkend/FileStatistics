#include <iostream>
#include <string>
#include <chrono>

#include "FileUtils.h"
#include "LineCounter.h"

int main()
{
    std::cout << "Enter root directory: " << std::endl;
    std::string root_directory;
    std::getline(std::cin, root_directory);

    

    {
        std::cout << "LineCounter not concurrent\n";
        auto startTime = std::chrono::high_resolution_clock::now();

        LineCounter lc;
        lc.ProcessDirectory(root_directory);

        auto endTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> executionTime = endTime - startTime;

        lc.SaveToStream(std::cout, executionTime);
        std::cout << std::endl;
    }

    {
        std::cout << "LineCounter concurrent\n";
        auto startTime = std::chrono::high_resolution_clock::now();

        LineCounter lc;
        lc.ProcessDirectoryConcurrently(root_directory);

        auto endTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> executionTime = endTime - startTime;

        lc.SaveToStream(std::cout, executionTime);
        std::cout << std::endl;
    }

    

}
     