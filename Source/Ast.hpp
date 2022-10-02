#pragma once

#include <memory>
#include <utility>
#include <variant>


template <typename ... F>
class overloaded : private F...
{
public:
    overloaded(F... f) : F(f)... {}
    using F::operator()...;
};


struct Stmt;
using Data_t = int32_t;

// struct Add
// {
//     std::shared_ptr<Expr> lhs, rhs;
// };

enum class Relop
{
    lower,
    leq,
    neq,
    geq,
    greater,
    equal
};

enum class Op
{
    plus,
    minus,
    mul,
    div
};

using Number = Data_t;
using Var = std::string;

struct Stmt
: std::variant<Number, Var, Relop, Op> 
{
    using variant::variant;
};

template <typename E>
auto MakeStmt(auto... e)
{
    return Stmt{E{std::make_shared<Stmt>(e)...}};
}