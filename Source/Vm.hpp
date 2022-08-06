#pragma once

#include "Ast.hpp"
#include "Compiler.hpp"

#include <span>
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


auto exec(const Chunk_type& c) -> Data_t
{
    std::stack<Data_t> stack;

    // std::cout << "size = " << sz << std::endl;
    const auto pop2 = [&]
    {
        const auto rhs = stack.top();
        stack.pop();
        const auto lhs = stack.top();
        stack.pop();
        return std::pair{lhs, rhs};
    };

    for(std::size_t pos = 0; pos < c.size(); ++pos)
    {
        const auto code = static_cast<std::byte>(c[pos]);

        switch(code)
        {
            case OpCode::Push:
            {                
                double value{};

                const std::span view{c.data() + pos + 1, sizeof(Data_t)};
                std::copy(view.begin(), view.end(), reinterpret_cast<std::uint8_t*>(&value)); 

                pos += sizeof(Data_t);
                stack.push(value);
                break;
            }

            case OpCode::Neg:
            {
                const auto operand = stack.top();
                stack.pop();
                stack.push(-operand);
                break;
            }

            case OpCode::Add:
            {
                const auto operands = pop2();
                stack.push(operands.first + operands.second);
                break;
            }

            case OpCode::Sub:
            {
                const auto operands = pop2();
                stack.push(operands.first - operands.second);
                break;
            }

            case OpCode::Div:
            {
                const auto operands = pop2();
                stack.push(operands.first / operands.second);
                break;
            }

            case OpCode::Mul:
            {

                const auto operands = pop2();
                stack.push(operands.first * operands.second);
                break;
            }

            case OpCode::Return: return stack.top();
            default:break;
        }
    }

    return {};
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
