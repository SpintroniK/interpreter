#include "Parser.hpp"
#include "Vm.hpp"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <chrono>
#include <numeric>
#include <ranges>
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

auto getDepth(const auto& ast, std::size_t depth = 0) -> std::size_t
{
            // <Data_t, Add, Sub, Mul, Div, Neg>
    return std::visit(overloaded
            {
                [&](Data_t) { return depth; },
                [&](const Neg& n) { depth = getDepth(*n.expr, depth + 1); return depth; },
                [&](const Mul& m) { depth = std::max(getDepth(*m.lhs, depth + 1), getDepth(*m.rhs, depth + 1)); return depth; },
                [&](const Div& m) { depth = std::max(getDepth(*m.lhs, depth + 1), getDepth(*m.rhs, depth + 1)); return depth; },
                [&](const Add& m) { depth = std::max(getDepth(*m.lhs, depth + 1), getDepth(*m.rhs, depth + 1)); return depth; },
                [&](const Sub& m) { depth = std::max(getDepth(*m.lhs, depth + 1), getDepth(*m.rhs, depth + 1)); return depth; },
            }, ast);
}

void print(const Expr& ast, std::string prefix = "", bool isLeft = false)
{
    struct ExprFmt
    {
        Expr e;
        std::string prefix;
        bool isNodeLeft;
        bool isLeft;   
    };
    
    const auto printNodes = [](const auto&... ef) 
    {
        (print(ef.e, ef.prefix + (ef.isNodeLeft ? "│   " : "    "), ef.isLeft), ...);
    };

    const auto printNode = [](const std::string& prefix, const std::string& symbol, bool isLeft)
    {
        std::cout << prefix;
        std::cout << (isLeft ? "├──" : "└──" );
        std::cout << symbol << std::endl;
    };

    const auto printLeaf = [](const std::string& prefix, bool isLeft, const auto& value)
    {
        std::cout << prefix << (isLeft ? "├──🍁 " : "└──🍁 " ) << value << std::endl;
    };

    // <Data_t, Add, Sub, Mul, Div, Neg>
    std::visit(overloaded
    {
        [&](Data_t value) 
        { 
            printLeaf(prefix, isLeft, value);
        },
        [&](const Neg& n) 
        {
            printNode(prefix, "➖", isLeft);
            printNodes(ExprFmt{*n.expr, prefix, isLeft, false});
        },
        [&](const Mul& m) 
        {
            printNode(prefix, "✖", isLeft);
            printNodes(ExprFmt{*m.lhs, prefix, isLeft, true}, 
                       ExprFmt{*m.rhs, prefix, isLeft, false});
        },
        [&](const Div& m) 
        {
            printNode(prefix, "➗", isLeft);
            printNodes(ExprFmt{*m.lhs, prefix, isLeft, true}, 
                       ExprFmt{*m.rhs, prefix, isLeft, false});
        },
        [&](const Add& m) 
        {
            printNode(prefix, "➕", isLeft);
            printNodes(ExprFmt{*m.lhs, prefix, isLeft, true}, 
                       ExprFmt{*m.rhs, prefix, isLeft, false});
        },
        [&](const Sub& m) 
        {
            printNode(prefix, "➖", isLeft);
            printNodes(ExprFmt{*m.lhs, prefix, isLeft, true}, 
                       ExprFmt{*m.rhs, prefix, isLeft, false});
        },
        [](auto ){}
    }, ast);
}

int main(int argc, char** argv)
{

    const auto args = std::vector<std::string_view>(argv, argv + argc);

    const auto isDebug = std::ranges::find(args, "-d") != args.end();

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

        const auto bytecode = _compile(parsed->first);   // 💻

        const auto bc = compile(parsed->first);
        // std::cout << bc << std::endl;

        Vm vm{bc};

        const auto res = vm.Execute();
        std::cout << "res = " << res << std::endl;

        std::ofstream out{"out.hex", std::ofstream::binary};
        out << bc;

        std::ifstream file{"in.hex", std::ifstream::binary};
        std::string bytec;

        file >> bytec;

        std::cout << "result = " << exec(bc) << std::endl;
        std::cout << "result [file] = " << exec(bytec) << std::endl;

        const auto astResult = eval(parsed->first);     // 🌳
        std::cout << "🌳 " << astResult << std::endl;

        const auto result = execute(bytecode);          // 💻
        std::cout << "💻 " << result << std::endl;


        if(isDebug)
        {
            // const auto d = getDepth(parsed->first);
            // std::cout << "↧ " << d << std::endl;
            print(parsed->first);                       // 🐞🌳
            debug(bytecode);                            // 🐞
        }

        if(!parsed->second.empty())
        {
            std::cout << "🤯 Cannot parse the following input: '" << parsed->second << "'" << std::endl;
        }

    }

    return EXIT_SUCCESS;
}
