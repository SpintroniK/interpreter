#include "Parser.hpp"

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
            return std::accumulate(vsf.begin(), vsf.end(), MakeExpr<Add>(f, Data_t{0}), [](const auto& acc, const auto& v)
            {
                return std::get<std::string>(v.first) == "-" ? MakeExpr<Sub>(acc, v.second) : MakeExpr<Add>(acc, v.second);
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
                    estr("-"),
                    estr("+")
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
            return std::accumulate(vsu.begin(), vsu.end(), MakeExpr<Mul>(u, Data_t{1}), [](const auto& acc, const auto& v)
            {
                return std::get<std::string>(v.first) == "/" ? MakeExpr<Div>(acc, v.second) : MakeExpr<Mul>(acc, v.second);
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
                    estr("/"),
                    estr("*")
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
            estr("-"),
            unary
        ),
        primary
    )(input);
}

// primary        → real | integer | "true" | "false" | "nil" | "(" expression ")" ;

auto primary(std::string_view input) -> Parsed
{
    return token
    (
        either
        (
            real,
            einteger,
            estr("true"),
            estr("false"),
            estr("nil"),
            sequence
            (
                [] (auto, auto e, auto) { return e;},
                estr("("),
                expression,
                estr(")")
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
            return v;
        },
        either
        (
            sequence
            (
                [](auto i, auto, auto f)
                {

                    const auto str = std::to_string(static_cast<int>(std::get<Data_t>(i))) 
                                     + "." 
                                     + (f? std::to_string(static_cast<int>(std::get<Data_t>(*f))) : "0");

                    return Expr{std::stod(str)}; 
                },
                einteger,
                symbol('.'),
                maybe(einteger)
            ),
            sequence(
                [](auto, const auto& x){ return Expr{std::stod("." + std::to_string(std::get<Data_t>(x)))}; },
                symbol('.'),
                einteger
            )
        )
    )(input);
}
