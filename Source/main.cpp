#include "Parser.hpp"

#include <iostream>
#include <string>
#include <string_view>
#include <vector>

int main(int argc, char** argv)
{

    const auto args = std::vector(argv, argv + argc);


    for(;;)
    {
        std::cout << "> ";

        std::string line;
        std::getline(std::cin, line);
        const auto input = std::string_view{line};

        if(input == "exit")
        {
            return EXIT_SUCCESS;
        }

        auto result = expr(input);

        if(result)
        {
            std::cout << result->first << std::endl;
        }
        else
        {
            std::cout << "Error, cannot parse input." << std::endl;
        }

    }


    return EXIT_SUCCESS;
}
