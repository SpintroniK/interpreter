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


struct Expr;

// struct Add
// {
//     std::shared_ptr<Expr> lhs, rhs;
// };




using Data_t = int32_t;
using Variant_t = std::variant<Data_t, std::string>;

struct Expr : Variant_t 
{
    using variant::variant;
};

template <typename E>
auto MakeExpr(auto... e)
{
    return Expr{E{std::make_shared<Expr>(e)...}};
}