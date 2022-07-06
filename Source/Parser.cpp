#include "Parser.hpp"

#include <functional>
#include <iostream>
#include <numeric>
#include <string>

using namespace std::string_literals;

// expression     → equality ;

auto expression(std::string_view input) -> Parsed
{
    return equality(input);
}

// equality       → comparison { ( "!=" | "==" ) comparison } ;

auto equality(std::string_view input) -> Parsed
{
    return sequence
    (
        [](auto c, auto) 
        { 
            return c; 
        },
        comparison,
        repeat
        (
            sequence
            (
                [](auto s, auto c) { return std::pair{s, c}; },
                either
                (
                    estr("!="),
                    estr("==")
                ),
                comparison
            )
        )
    )(input);
}

// comparison     → term { ( ">" | ">=" | "<" | "<=" ) term } ;

auto comparison(std::string_view input) -> Parsed
{
    return sequence
    (
        [] (auto t, auto)
        {
    
            // const auto res = std::visit(overloaded
            // {
            //     [&](int x, int y) -> Variant_t 
            //     {
            //         const auto op = vst.front().first;
            //         return x > y ? "true"s : "false"s; 
            //     },
            //     [](auto, auto) -> Variant_t { return "false"s; }
            // }
            // , t.first, vst.front().second.first);

            // return MakeNode(res);

            return t;
        },
        term,
        repeat
        (
            sequence
            (
                [] (auto s, auto t) { return std::pair{s, t}; },
                either
                (
                    estr(">"),
                    estr(">="),
                    estr("<"),
                    estr("<=")
                ),
                term
            )
        )
    )(input);
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
                // if(std::get<std::string>(s) == "!")
                // {
                //     return Expr{"false"s};
                // }
                // else
                {
                    return MakeExpr<Neg>(u);
                }
            },
            either
            (
                estr("!"),
                estr("-")
            ),
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
