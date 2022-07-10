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

namespace OpCodes
{
    struct NoOp {};
    struct Push { Data_t value{}; };
    struct Return {};
    struct Neg { Data_t lhs{}, rhs{}; };
    struct Add { Data_t lhs{}, rhs{}; };
    struct Sub { Data_t lhs{}, rhs{}; };
    struct Mul { Data_t lhs{}, rhs{}; };
    struct Div { Data_t lhs{}, rhs{}; };

    struct Code : std::variant<NoOp, Push, Return, Neg, Add, Sub, Mul, Div>
    {
        using variant::variant;
    };
}


// using OpCodes = std::tuple<uint8_t, Data_t>;
using Chunk_t = std::vector<OpCodes::Code>;

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
        [](Data_t value) -> OpCodes::Code { return OpCodes::Push{value}; },
        [&](const Neg& n) -> OpCodes::Code
        { 
            newChunk = compileExpr(*n.expr, c);
            return OpCodes::Neg{}; 
        },
        [&](const Add& n) -> OpCodes::Code
        {
            newChunk = compileExpressions(newChunk, *n.lhs, *n.rhs);
            return OpCodes::Add{}; 
        },
        [&](const Mul& n) -> OpCodes::Code
        {
            newChunk = compileExpressions(newChunk, *n.lhs, *n.rhs);
            return OpCodes::Mul{}; 
        },
        [&](const Sub& n) -> OpCodes::Code
        { 
            newChunk = compileExpressions(newChunk, *n.lhs, *n.rhs);
            return OpCodes::Sub{}; 
        },
        [&](const Div& n) -> OpCodes::Code
        { 
            newChunk = compileExpressions(newChunk, *n.lhs, *n.rhs);
            return OpCodes::Div{}; 
        },
        [](auto) -> OpCodes::Code { return OpCodes::NoOp{}; },
    }, ast);

    newChunk.push_back(r);

    return newChunk;
}


auto compile(const auto& ast) -> Chunk_t
{
    Chunk_t c;
    return compileExpr(ast, c);
}