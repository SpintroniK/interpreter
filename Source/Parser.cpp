#include "Parser.hpp"
#include "Ast.hpp"

#include <functional>
#include <iostream>
#include <numeric>
#include <string>

using namespace std::string_literals;

// line = number statement | statement .
auto line(std::string_view input) -> Parsed
{
    return either
    (
        sequence
        (
            [](auto, auto) { return std::string{}; },
            number,
            statement
        ),
        statement
    )(input);
}

// statement = "PRINT" expr-list | "IF" expression relop expression "THEN" statement | "GOTO" expression | "INPUT" var-list |
// "LET" var "=" expression | "GOSUB" expression | "RETURN" | "CLEAR" | "LIST" | "RUN" | "END" .
auto statement(std::string_view input) -> Parsed
{
    return either
    (
        sequence
        (
            [](auto, auto){ return std::string{}; },
            str("PRINT"),
            expr_list
        ),
        sequence
        (
            [](auto, auto, auto, auto, auto, auto) { return std::string{}; },
            str("IF"),
            expression,
            relop,
            expression,
            str("THEN"),
            statement
        ),
        sequence
        (
            [](auto, auto) { return std::string{}; },
            str("GOTO"),
            expression
        ),
        sequence
        (
            [](auto, auto) { return std::string{}; },
            str("INPUT"),
            var_list
        ),
        sequence
        (
            [](auto, auto, auto, auto) { return std::string{}; },
            str("LET"),
            var,
            str("="),
            expression
        ),
        sequence
        (
            [](auto, auto) { return std::string{}; },
            str("GOSUB"),
            expression
        ),
        str("RETURN"),
        str("CLEAR"),
        str("LIST"),
        str("RUN"),
        str("END")
    )(input);
}

// expr-list = (string | expression) {"," (string | expression) } .
auto expr_list(std::string_view input) -> Parsed
{
    return sequence
    (
        [](auto, auto) { return std::string{}; },
        either
        (
            str(" "),
            expression
        ),
        repeat
        (
            sequence
            (
                [](auto, auto) { return std::string{}; },
                str(","),
                either
                (
                    str(" "),
                    expression
                )
            )
        )
    )(input);
}

// var-list = var {"," var} .
auto var_list(std::string_view input) -> Parsed
{
    return sequence
    (
        [](auto, auto) { return std::string{}; },
        var,
        repeat
        (
            sequence
            (
                [](auto, auto) { return std::string{}; },
                symbol(','),
                var
            )
        )
    )(input);
}

// expression = ("+" | "-" | e) term {("+" | "-") term} .
auto expression(std::string_view input) -> Parsed
{
    return sequence
    (
        [](auto, auto, auto) { return std::string{}; },
        maybe
        (
            either
            (
                str("+"),
                str("-")
            )
        ),
        term,
        repeat
        (
            sequence
            (
                [](auto, auto) { return std::string{}; },
                either
                (
                    str("+"),
                    str("-")
                ),
                term
            )
        )
    )(input);
}

// term = factor {("*" | "/") factor} .
auto term(std::string_view input) -> Parsed
{
    return sequence
    (
        [](auto, auto) { return std::string{}; },
        factor,
        repeat
        (
            sequence
            (
                [](auto, auto)  { return std::string{}; },
                either
                (
                    symbol('*'),
                    symbol('/')
                ),
                factor
            )
        )
    )(input);
}

// factor = var | number | (expression) .
auto factor(std::string_view input) -> Parsed
{
    return 
    either
    (
        var,
        number,
        sequence
        (
            [](auto, auto, auto) { return Expr{std::string{}}; },
            symbol('('),
            expression,
            symbol(')')
        )
    )(input);
}

// var = "A" | "B" | "C" | "D" | "E" | "F" | "G" | "H" | "I" | "J" | "K" | "L" | "M" | "N" | "O" | "P" | "Q" | "R" | "S" | "T" | "U" | "V" | "W" | "X" | "Y" | "Z" . 
auto var(std::string_view input) -> Parsed
{
    return chain(upper, [](auto ){ return unit(Expr{std::string{}}); })(input);
}

// number = digit { digit } .
auto number(std::string_view input) -> Parsed
{
    return chain(integer, [](auto ){ return unit(Expr{std::string{}}); })(input);
}

auto relop(std::string_view input) -> Parsed
{
    return token
    (
        either
        (
            str("<"),
            str("<="),
            str("<>"),
            str("><"),
            str(">="),
            str(">"),
            str("=")
        )
    )(input);
}