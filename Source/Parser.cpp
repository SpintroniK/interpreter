#include "Parser.hpp"
#include "Ast.hpp"

#include <functional>
#include <iostream>
#include <numeric>
#include <string>

using namespace std::string_literals;

// expression     → equality ;

auto expression(std::string_view input) -> Parsed
{
    return term(input);
}

// term           → factor { ( "-" | "+" ) factor } ;

auto term(std::string_view input) -> Parsed
{
    return sequence
    (
        [] (auto f, auto vsf) 
        {
            return std::accumulate(vsf.begin(), vsf.end(), Expr{f}, [](const auto& acc, const auto& v)
            {
                return v.first == '-' ? MakeExpr<Sub>(acc, v.second) : MakeExpr<Add>(acc, v.second);
            });
        },
        factor,
        repeat
        (
            sequence
            (
                [] (auto s, auto f) { return std::pair{s, f}; },
                either
                (
                    symbol('-'),
                    symbol('+')
                ),
                factor
            )
        )
    )(input);
}

// factor         → unary { ( "/" | "*" ) unary } ;

auto factor(std::string_view input) -> Parsed
{
    return sequence
    (
        [] (auto u, auto vsu) 
        {
            return std::accumulate(vsu.begin(), vsu.end(), Expr{u}, [](const auto& acc, const auto& v)
            {
                return v.first == '/' ? MakeExpr<Div>(acc, v.second) : MakeExpr<Mul>(acc, v.second);
            });
        },
        unary,
        repeat
        (
            sequence
            (
                [] (auto s, auto u) { return std::pair{s, u}; },
                either
                (
                    symbol('/'),
                    symbol('*')
                ),
                unary
            )
        )
    )(input);
}

// unary          → ( "!" | "-" ) unary | primary ;

auto unary(std::string_view input) -> Parsed
{
    return either
    (
        sequence
        (
            [] (auto, auto u)
            {
                    return MakeExpr<Neg>(u);
            },
            symbol('-'),
            unary
        ),
        primary
    )(input);
}

// primary        → real | integer | "(" expression ")" ;

auto primary(std::string_view input) -> Parsed
{
    return token
    (
        either
        (
            real,
            chain(integer, [](auto i) { return unit(Expr{i}); }),
            sequence
            (
                [] (auto, auto e, auto) { return e;},
                symbol('('),
                expression,
                symbol(')')
            )
        )
    )(input);
}

// real = integer "." [integer] | "." integer.
auto real(std::string_view input) -> Parsed
{
    return sequence
    (
        [](auto v)
        {
            return Expr{v};
        },
        either
        (
            sequence
            (
                [](auto i, auto, auto f)
                {

                    const auto str = std::to_string(static_cast<int>(i)) 
                                     + "." 
                                     + (f? std::to_string(static_cast<int>(*f)) : "0");

                    return std::stod(str); 
                },
                integer,
                symbol('.'),
                maybe(integer)
            ),
            sequence(
                [](auto, const auto& x){ return std::stod("." + std::to_string(x)); },
                symbol('.'),
                integer
            )
        )
    )(input);
}
