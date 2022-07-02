#include "Parser.hpp"

#include <functional>
#include <iostream>
#include <numeric>

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
        [](auto c, auto) -> Value_t { return c; },
        comparison,
        repeat
        (
            sequence
            (
                [](auto s, auto c) { return std::pair{s, c}; },
                either
                (
                    str("!="),
                    str("==")
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
        [] (auto t, auto vst) -> Value_t
        {
            auto d = t;
            for(const auto& st : vst)
            {
                if(st.first == ">") d = d > st.second;
                if(st.first == ">=") d = d >= st.second;
                if(st.first == "<") d = d < st.second;
                if(st.first == "<=") d = d <= st.second;
            }

            return d;
        },
        term,
        repeat
        (
            sequence
            (
                [] (auto s, auto t) { return std::pair{s, t}; },
                either
                (
                    str(">"),
                    str(">="),
                    str("<"),
                    str("<=")
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
        [] (auto f, auto vsu) 
        {
            return std::accumulate(vsu.begin(), vsu.end(), f, [] (const auto& acc, const auto& v)
            {
                return v.first == '-' ? acc - v.second : acc + v.second;
            });
        },
        factor,
        repeat
        (
            sequence
            (
                [] (auto s, auto u) { return std::pair{s, u}; },
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
            return std::accumulate(vsu.begin(), vsu.end(), u, [] (const auto& acc, const auto& v)
            {
                return v.first == '/' ? acc / v.second : acc * v.second;
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
            [] (auto s, auto u)
            { 
                return s == '!' ? !u : -u; 
            },
            either
            (
                symbol('!'),
                symbol('-')
            ),
            unary
        ),
        primary
    )(input);
}

// primary        → integer | "true" | "false" | "nil" | "(" expression ")" ;

auto primary(std::string_view input) -> Parsed
{
    return token
    (
        either
        (
            integer,
            // str("true"),
            // str("false"),
            // str("nil"),
            sequence
            (
                [] (auto, auto e, auto) { return e;},
                str("("),
                expression,
                str(")")
            )
        )
    )(input);
}