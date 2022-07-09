#include "Parser.hpp"
#include "Ast.hpp"

#include <iostream>
#include <numeric>
#include <stack>
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

enum class OpCode : uint8_t
{
    NoOp = 0,
    Push,
    Return,
    Neg,
    Add,
    Sub,
    Mul,
    Div,
};


struct VNoOp {};
struct VPush { Data_t value{}; };
struct VReturn {};
struct VNeg { Data_t lhs{}, rhs{}; };
struct VAdd { Data_t lhs{}, rhs{}; };
struct VMul { Data_t lhs{}, rhs{}; };

struct VCode : std::variant<VNoOp, VPush, VReturn, VNeg, VAdd, VMul>
{
    using variant::variant;
};

using OpCodes = std::tuple<uint8_t, Data_t>;
using Chunk_t = std::vector<OpCodes>;
using ExprOpCodes_t = std::pair<Expr, OpCodes>;

template <typename... T>
auto compileExpressions(Chunk_t& c, const T&... expressions)
{
    std::common_type_t<T...> newAst;

    ([&]
    {
        const auto newRet = compileExpr(expressions, c);
        newAst = newRet.first;
        c = newRet.second;
    }(), ...);

    return newAst;
}

auto compileExpr(const auto& ast, const Chunk_t& c) -> std::pair<Expr, Chunk_t>
{
    auto newChunk = c;
    auto newAst = ast;
    const auto r = std::visit(overloaded
    {
        [](Data_t value) -> ExprOpCodes_t { return { {}, OpCodes{static_cast<uint8_t>(OpCode::Push), value} }; },
        [&](const Neg& n) -> ExprOpCodes_t
        { 
            const auto newRet = compileExpr(*n.expr, c);
            newAst = newRet.first;
            newChunk = newRet.second;
            return { newAst, OpCodes{static_cast<uint8_t>(OpCode::Neg), 0} }; 
        },
        [&](const Add& n) -> ExprOpCodes_t
        {
            return { compileExpressions(newChunk, *n.lhs, *n.rhs), OpCodes{static_cast<uint8_t>(OpCode::Add), 0} }; 
        },
        [&](const Mul& n) -> ExprOpCodes_t
        { 
            return { compileExpressions(newChunk, *n.lhs, *n.rhs), OpCodes{static_cast<uint8_t>(OpCode::Mul), 0} }; 
        },
        [&](const Sub& n) -> ExprOpCodes_t
        { 
            return { compileExpressions(newChunk, *n.lhs, *n.rhs), OpCodes{static_cast<uint8_t>(OpCode::Sub), 0} }; 
        },
        [&](const Div& n) -> ExprOpCodes_t
        { 
            return { compileExpressions(newChunk, *n.lhs, *n.rhs), OpCodes{static_cast<uint8_t>(OpCode::Div), 0} }; 
        },
        [](auto) -> ExprOpCodes_t { return { {}, OpCodes{static_cast<uint8_t>(OpCode::NoOp), 0} }; },
    }, ast);

    newChunk.push_back(r.second);

    return std::pair{newAst, newChunk};
}


auto compile(const auto& ast) -> Chunk_t
{
    Chunk_t c;
    return compileExpr(ast, c).second;
}

auto execute(const Chunk_t& c) -> Data_t
{
    std::stack<Data_t> s;
    for(const auto& op : c)
    {
        const auto opCode = std::get<0>(op);

        switch(static_cast<OpCode>(opCode))
        {
            case OpCode::Neg:
            {
                const auto v  = s.top();
                s.pop();
                s.push(-v);
                break;
            }
            case OpCode::Add:
            {
                const auto rhs = s.top();
                s.pop();
                const auto lhs = s.top();
                s.pop();
                s.push(lhs + rhs);
                break;
            }
            case OpCode::Sub:
            {
                const auto rhs = s.top();
                s.pop();
                const auto lhs = s.top();
                s.pop();
                s.push(lhs - rhs);
                break;
            }
            case OpCode::Mul:
            {
                const auto rhs = s.top();
                s.pop();
                const auto lhs = s.top();
                s.pop();
                s.push(lhs * rhs);
                break;
            }
            case OpCode::Div:
            {
                const auto rhs = s.top();
                s.pop();
                const auto lhs = s.top();
                s.pop();
                s.push(lhs / rhs);
                break;
            }
            case OpCode::Push: s.push(std::get<1>(op)); break;
            
            default:
                break;
        }
    }

    return s.top();
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
            return EXIT_SUCCESS;
        }

        const auto parsed = expression(input);

        if(parsed)
        {
            const auto bytecode = compile(parsed->first);
            // for(const auto& b : bytecode)
            // {
            //     std::cout << "> " << +std::get<0>(b) << " | " << +std::get<1>(b) << std::endl;
            // }
            const auto result = execute(bytecode);
            std::cout << "💻 " << result << std::endl;
            std::cout << "✅ " << eval(parsed->first) << std::endl;
        }
        else
        {
            std::cout << "Error, cannot parse input." << std::endl;
        }

    }


    return EXIT_SUCCESS;
}
