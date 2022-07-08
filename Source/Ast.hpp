#pragma once

#include <utility>
#include <variant>
#include <vector>


template <typename ... F>
class overloaded : private F...
{
public:
    overloaded(F... f) : F(f)... {}
    using F::operator()...;
};


struct Expr;

struct Add
{
    std::shared_ptr<Expr> lhs, rhs;
};

struct Sub
{
    std::shared_ptr<Expr> lhs, rhs;
};

struct Mul
{
    std::shared_ptr<Expr> lhs, rhs;
};

struct Div
{
    std::shared_ptr<Expr> lhs, rhs;
};

struct Neg
{
    std::shared_ptr<Expr> expr;
};


using Data_t = double;
using Variant_t = std::variant<Data_t, Add, Sub, Mul, Div, Neg>;

struct Expr : Variant_t 
{
    using variant::variant;
};

template <typename E>
auto MakeExpr(auto... e)
{
    return Expr{E{std::make_shared<Expr>(e)...}};
}