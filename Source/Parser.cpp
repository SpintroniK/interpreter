#include "Parser.hpp"

#include <iostream>

// expr = term ("+" | "-") expr | term.
auto expr(std::string_view input) -> Parsed
{
    return either
    (
        sequence
        (
            [](auto x, auto c, auto y) -> Value_t { return c == '+' ? x + y : x - y; },
            term,
            either
            (
                symbol('+'),
                symbol('-')
            ),
            expr
        ),
        term
    )(input);
}

// term = factor ("*" | "/") term | factor.
auto term(std::string_view input) -> Parsed
{
    return either
    (
        sequence
        (
            [](auto x, auto c, auto y) -> Value_t { return c == '*' ? x * y : x / y; },
            factor,
            either
            (
                symbol('*'),
                symbol('/')
            ),
            term
        ),
        factor
    )(input);
}

// factor = "(" expr ")" | integer.
auto factor(std::string_view input) -> Parsed
{
    return token
    (
        either
        (
            sequence
            (
                [](auto, auto x, auto) -> Value_t { return x; },
                symbol('('),
                expr,
                symbol(')')
            ),
            integer
        )
    )(input);
}

