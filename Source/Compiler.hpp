#pragma once

#include <tuple>
#include <variant>
#include <vector>

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

namespace Codes
{
    struct NoOp {};
    struct Push { Data_t value{}; };
    struct Return {};
    struct Neg { Data_t lhs{}, rhs{}; };
    struct Add { Data_t lhs{}, rhs{}; };
    struct Mul { Data_t lhs{}, rhs{}; };

    struct Code : std::variant<NoOp, Push, Return, Neg, Add, Mul>
    {
        using variant::variant;
    };
}


using OpCodes = std::tuple<uint8_t, Data_t>;
using Chunk_t = std::vector<OpCodes>;

auto compileExpressions(const Chunk_t& c, const auto&... expressions)
{
    Chunk_t newChunk = c;

    ((newChunk = compileExpr(expressions, newChunk)), ...);

    return newChunk;
}

auto compileExpr(const auto& ast, const Chunk_t& c) -> Chunk_t
{
    auto newChunk = c;
    const auto r = std::visit(overloaded
    {
        [](Data_t value) { return OpCodes{static_cast<uint8_t>(OpCode::Push), value}; },
        [&](const Neg& n)
        { 
            newChunk = compileExpr(*n.expr, c);
            return OpCodes{static_cast<uint8_t>(OpCode::Neg), 0}; 
        },
        [&](const Add& n)
        {
            newChunk = compileExpressions(newChunk, *n.lhs, *n.rhs);
            return OpCodes{static_cast<uint8_t>(OpCode::Add), 0}; 
        },
        [&](const Mul& n)
        {
            newChunk = compileExpressions(newChunk, *n.lhs, *n.rhs);
            return OpCodes{static_cast<uint8_t>(OpCode::Mul), 0}; 
        },
        [&](const Sub& n)
        { 
            newChunk = compileExpressions(newChunk, *n.lhs, *n.rhs);
            return OpCodes{static_cast<uint8_t>(OpCode::Sub), 0}; 
        },
        [&](const Div& n)
        { 
            newChunk = compileExpressions(newChunk, *n.lhs, *n.rhs);
            return OpCodes{static_cast<uint8_t>(OpCode::Div), 0}; 
        },
        [](auto) { return OpCodes{static_cast<uint8_t>(OpCode::NoOp), 0}; },
    }, ast);

    newChunk.push_back(r);

    return newChunk;
}


auto compile(const auto& ast) -> Chunk_t
{
    Chunk_t c;
    return compileExpr(ast, c);
}