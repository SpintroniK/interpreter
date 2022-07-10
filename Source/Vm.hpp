#pragma once

#include "Ast.hpp"
#include "Compiler.hpp"

#include <stack>

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