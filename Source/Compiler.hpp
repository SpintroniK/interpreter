#pragma once

#include <iostream>
#include <tuple>
#include <variant>
#include <vector>

// enum class OpCode : uint8_t
// {
//     NoOp = 0,
//     Push,
//     Return,
//     Neg,
//     Add,
//     Sub,
//     Mul,
//     Div,
// };

struct OpCode
{
    static constexpr std::byte NoOp{0x00};
    static constexpr std::byte Push{0x01}; 
    static constexpr std::byte Return{0x02};
    static constexpr std::byte Neg{0x03};  
    static constexpr std::byte Add{0x04};
    static constexpr std::byte Sub{0x05};  
    static constexpr std::byte Mul{0x06};
    static constexpr std::byte Div{0x07};
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
using Chunk_type = std::string;

auto _compileExpressions(const Chunk_t& c, const auto&... expressions)
{
    Chunk_t newChunk = c;

    ((newChunk = _compileExpr(expressions, newChunk)), ...);

    return newChunk;
}

auto _compileExpr(const auto& ast, const Chunk_t& c) -> Chunk_t
{
    auto newChunk = c;
    const auto r = std::visit(overloaded
    {
        [](Data_t value) -> OpCodes::Code { return OpCodes::Push{value}; },
        [&](const Neg& n) -> OpCodes::Code
        { 
            newChunk = _compileExpr(*n.expr, c);
            return OpCodes::Neg{}; 
        },
        [&](const Add& n) -> OpCodes::Code
        {
            newChunk = _compileExpressions(newChunk, *n.lhs, *n.rhs);
            return OpCodes::Add{}; 
        },
        [&](const Mul& n) -> OpCodes::Code
        {
            newChunk = _compileExpressions(newChunk, *n.lhs, *n.rhs);
            return OpCodes::Mul{}; 
        },
        [&](const Sub& n) -> OpCodes::Code
        { 
            newChunk = _compileExpressions(newChunk, *n.lhs, *n.rhs);
            return OpCodes::Sub{}; 
        },
        [&](const Div& n) -> OpCodes::Code
        { 
            newChunk = _compileExpressions(newChunk, *n.lhs, *n.rhs);
            return OpCodes::Div{}; 
        },
        [](auto) -> OpCodes::Code { return OpCodes::NoOp{}; },
    }, ast);

    newChunk.push_back(r);

    return newChunk;
}


auto _compile(const auto& ast) -> Chunk_t
{
    Chunk_t c;
    return _compileExpr(ast, c);
}

auto compileExpressions(const Chunk_type& c, const auto&... expressions)
{
    Chunk_type newChunk = c;

    ((newChunk += compileExpr(expressions, newChunk)), ...);

    return newChunk;
}

auto compileExpr(const auto& ast, const Chunk_type& c) -> Chunk_type
{
    auto newChunk = c;
    const auto r = std::visit(overloaded
    {
        [](Data_t value) -> std::string 
        { 
            const auto op = static_cast<char>(OpCode::Push);
            return op + std::string{reinterpret_cast<char*>(&value), sizeof(Data_t)};
        },
        [&](const Neg& e) -> std::string
        {
            newChunk = compileExpr(*e.expr, newChunk);
            return {static_cast<char>(OpCode::Neg)};
        },
        [&](const Add& e) -> std::string
        {
            newChunk = compileExpressions(newChunk, *e.lhs, *e.rhs);
            return {static_cast<char>(OpCode::Add)};
        },
        [&](const Mul& e) -> std::string
        {
            newChunk = compileExpressions(newChunk, *e.lhs, *e.rhs);
            return {static_cast<char>(OpCode::Mul)};
        },
        [&](const Sub& e) -> std::string
        { 
            newChunk = compileExpressions(newChunk, *e.lhs, *e.rhs);
            return {static_cast<char>(OpCode::Sub)};
        },
        [&](const Div& e) -> std::string
        { 
            newChunk = compileExpressions(newChunk, *e.lhs, *e.rhs);
            return {static_cast<char>(OpCode::Div)};
        },
        // [](auto) -> std::string 
        // { 
        //     return {}; 
        // },
    }, ast);

    return newChunk + r;
}

auto compile(const auto& ast) -> Chunk_type
{
    const Chunk_type c;
    const auto bytecode = compileExpr(ast, c);
    return bytecode + static_cast<char>(OpCode::Return);
}