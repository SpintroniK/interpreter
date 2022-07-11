#pragma once

#include "Ast.hpp"
#include "Compiler.hpp"

#include <iostream>
#include <stack>
#include <variant>

auto execute(const Chunk_t& c) -> Data_t
{
    std::stack<Data_t> s;

    for(const auto& op : c)
    {
        std::visit(overloaded
        {
            [&](OpCodes::Neg)
            {
                const auto v  = s.top();
                s.pop();
                s.push(-v);
            },
            [&](OpCodes::Add)
            {
                const auto rhs = s.top();
                s.pop();
                const auto lhs = s.top();
                s.pop();
                s.push(lhs + rhs);
            },
            [&](OpCodes::Sub)
            {
                const auto rhs = s.top();
                s.pop();
                const auto lhs = s.top();
                s.pop();
                s.push(lhs - rhs);
            },
            [&](OpCodes::Mul)
            {
                const auto rhs = s.top();
                s.pop();
                const auto lhs = s.top();
                s.pop();
                s.push(lhs * rhs);
            },
            [&](OpCodes::Div)
            {
                const auto rhs = s.top();
                s.pop();
                const auto lhs = s.top();
                s.pop();
                s.push(lhs / rhs);
            },
            [&](const OpCodes::Push& v)
            {
                s.push(v.value);
            },
            [&](auto) {},
        }, op);
    }

    return s.top();
}


auto debug(const Chunk_t& c)
{

    std::cout << "┌────────────────┐" << std::endl;
    std::cout << "│ 🐞 Stack trace │" << std::endl;
    std::cout << "└────────────────┘" << std::endl;


    std::cout << "┌────────────────┐" << std::endl;

    for(const auto& op : c)
    {
        std::visit(overloaded
        {
            [&](OpCodes::Neg) { std::cout << "│➖ Negate       │   " << std::endl; },
            [&](OpCodes::Add) { std::cout << "│➕ Add          │   " << std::endl; },
            [&](OpCodes::Sub) { std::cout << "│➖ Subtract     │   " << std::endl; },
            [&](OpCodes::Mul) { std::cout << "│ ✖ Multiply     │   " << std::endl; },
            [&](OpCodes::Div) { std::cout << "│➗ Divide       │   " << std::endl; },
            [&](const OpCodes::Push& v) { std::cout << "│📌 Push         │ 💾 " << v.value << std::endl; },
            [&](auto) {},
        }, op);
    }

    std::cout << "└────────────────┘" << std::endl;
}
