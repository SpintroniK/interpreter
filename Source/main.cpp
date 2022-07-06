#include "Parser.hpp"


#include <iostream>
#include <numeric>
#include <string>
#include <string_view>
#include <vector>

using Result_t = std::variant<Data_t, std::string>;

auto eval(const Value_t& ast) -> Result_t
{
            // <Data_t, std::string, Add, Sub, Mul, Div, Neg>
    return std::visit(overloaded
            {
                [](Data_t value) -> Result_t { return value; },
                [](const Neg& n) -> Result_t { return -std::get<Data_t>(eval(*n.expr)); },
                [](const Mul& m) -> Result_t { return std::get<Data_t>(eval(*m.lhs)) * std::get<Data_t>(eval(*m.rhs));},
                [](const Div& m) -> Result_t { return std::get<Data_t>(eval(*m.lhs)) / std::get<Data_t>(eval(*m.rhs));},
                [](const Add& m) -> Result_t { return std::get<Data_t>(eval(*m.lhs)) + std::get<Data_t>(eval(*m.rhs));},
                [](const Sub& m) -> Result_t { return std::get<Data_t>(eval(*m.lhs)) - std::get<Data_t>(eval(*m.rhs));},
                [](const std::string& s) -> Result_t { return s; },
                [](auto) -> Result_t { return Data_t{};}
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
            std::visit(overloaded
            {
                [](const std::string& s) { std::cout << s << std::endl; },
                [](const Data_t& val) { std::cout << val << std::endl; },
            }, eval(parsed->first));
        }
        else
        {
            std::cout << "Error, cannot parse input." << std::endl;
        }

    }


    return EXIT_SUCCESS;
}
