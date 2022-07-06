#include "Parser.hpp"
#include "Ast.hpp"

#include <iostream>
#include <numeric>
#include <string>
#include <string_view>
#include <vector>

auto eval(const auto& ast) -> Data_t
{
            // <Data_t, Add, Sub, Mul, Div, Neg>
    return std::visit(overloaded
            {
                [](Data_t value) { return value; },
                [](const Neg& n) { return -eval(*n.expr); },
                [](const Mul& m) { return eval(*m.lhs) * eval(*m.rhs); },
                [](const Div& m) { return eval(*m.lhs) / eval(*m.rhs); },
                [](const Add& m) { return eval(*m.lhs) + eval(*m.rhs); },
                [](const Sub& m) { return eval(*m.lhs) - eval(*m.rhs); },
                [](auto) { return Data_t{};}
            }, ast);
}

int main(int argc, char** argv)
{

    const auto args = std::vector(argv, argv + argc);

    for(;;)
    {
        std::cout << "🖉  ";

        std::string line;
        std::getline(std::cin, line);
        const auto input = std::string_view{line};

        if(input == "exit")
        {
            return EXIT_SUCCESS;
        }

        const auto parsed = expression(input);

        if(parsed)
        {
            std::cout << eval(parsed->first) << std::endl;
        }
        else
        {
            std::cout << "Error, cannot parse input." << std::endl;
        }

    }


    return EXIT_SUCCESS;
}
