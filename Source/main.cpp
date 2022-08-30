#include "Ast.hpp"
#include "Parser.hpp"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <chrono>
#include <numeric>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

auto eval(const auto& ast) -> void
{
            // <Data_t, Add, Sub, Mul, Div, Neg>
    return std::visit(overloaded
            {
                [](Data_t) {},
                [](auto){}
            }, ast);
}


// void print(const Expr& ast, std::string prefix = "", bool isLeft = false)
// {
//     struct ExprFmt
//     {
//         Expr e;
//         std::string prefix;
//         bool isNodeLeft;
//         bool isLeft;   
//     };
    
//     const auto printNodes = [](const auto&... ef) 
//     {
//         (print(ef.e, ef.prefix + (ef.isNodeLeft ? "│   " : "    "), ef.isLeft), ...);
//     };

//     const auto printNode = [](const std::string& prefix, const std::string& symbol, bool isLeft)
//     {
//         std::cout << prefix;
//         std::cout << (isLeft ? "├──" : "└──" );
//         std::cout << symbol << std::endl;
//     };

//     const auto printLeaf = [](const std::string& prefix, bool isLeft, const auto& value)
//     {
//         std::cout << prefix << (isLeft ? "├──🍁 " : "└──🍁 " ) << value << std::endl;
//     };


//     // std::visit(overloaded
//     // {
//     // }, ast);
// }

int main(int argc, char** argv)
{

    const auto args = std::vector<std::string_view>(argv, argv + argc);

    const auto isDebug = std::ranges::find(args, "-d") != args.end();

    for(;;)
    {
        std::cout << "📝  ";

        std::string inputLine;
        std::getline(std::cin, inputLine);
        const auto input = std::string_view{inputLine};

        if(input == "q")
        {
            std::cout << "💬 See you!" << std::endl;
            return EXIT_SUCCESS;
        }

        const auto parsed = line(input);  // 🌳

        if(!parsed)
        {
            std::cout << "😟 Error: cannot parse input." << std::endl;
            continue;
        }




        eval(parsed->first);     // 🌳
        // std::cout << "🌳 " << astResult << std::endl;



        if(isDebug)
        {
            // const auto d = getDepth(parsed->first);
            // std::cout << "↧ " << d << std::endl;
        }

        if(!parsed->second.empty())
        {
            std::cout << "🤯 Cannot parse the following input: '" << parsed->second << "'" << std::endl;
        }

    }

    return EXIT_SUCCESS;
}
