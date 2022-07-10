#include "Parser.hpp"
#include "Vm.hpp"

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
            }, ast);
}



int main(int argc, char** argv)
{

    const auto args = std::vector(argv, argv + argc);

    for(;;)
    {
        std::cout << "📝  ";

        std::string line;
        std::getline(std::cin, line);
        const auto input = std::string_view{line};

        if(input == "q")
        {
            std::cout << "💬 See you!" << std::endl;
            return EXIT_SUCCESS;
        }

        const auto parsed = expression(input);  // 🌳

        if(!parsed)
        {
            std::cout << "😟 Error: cannot parse input." << std::endl;
            continue;
        }

        const auto bytecode = compile(parsed->first);   // 💻

        const auto astResult = eval(parsed->first);     // 🌳
        std::cout << "🌳 " << astResult << std::endl;

        const auto result = execute(bytecode);          // 💻
        std::cout << "💻 " << result << std::endl;

        debug(bytecode);                                // 🐞

        if(!parsed->second.empty())
        {
            std::cout << "🤯 Cannot parse the following input: '" << parsed->second << "'" << std::endl;
        }

    }

    return EXIT_SUCCESS;
}
