#include "Parser.hpp"

#include <iostream>



// expr = term ("+" | "-") expr | term.
auto expr(std::string_view input) -> Parsed
{
    return either
    (
        sequence
        (
            [](auto x, auto c, auto y) -> Value_t 
            { 
                return c == '+' ?  MakeNode(Plus{}, {x, y}) : MakeNode(Minus{}, {x, y}); 
            },
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
            [](auto x, auto c, auto y) -> Value_t { return c == '*' ? MakeNode(Mult{}, {x, y}) : MakeNode(Div{}, {x, y}); },
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


// // expression = equality.
// auto expression(std::string_view input)-> Parsed
// {
//     return equality(input);
// }

// // equality = comparison {("!=" | "==") comparison}.
// auto equality(std::string_view input) -> Parsed
// {
//     return sequence
//     (
//         [](auto, auto) -> ValueType { return {}; },
//         comparison,
//         repeat
//         (
//             sequence
//             (
//                 [](auto, auto) -> ValueType { return {}; },            
//                 either
//                 (
//                     str("!="),
//                     str("==")
//                 ),
//                 comparison
//             )
//         )
//     )(input);
// }

// // comparison = term {( ">" | ">=" | "<" | "<=" ) term}.
// auto comparison(std::string_view input) -> Parsed
// {
//     return sequence
//     (
//         [](auto, auto) -> ValueType { return {}; },
//         term,
//         repeat
//         (
//             sequence
//             (
//                 [](auto, auto) -> ValueType { return {}; },
//                 (
//                     either
//                     (
//                         str(">"),
//                         str(">="),
//                         str("<"),
//                         str("<=")
//                     )
//                 ),
//                 term
//             )
//         )
//     )(input);
// }

// auto term(std::string_view input) -> Parsed
// {
//     return sequence
//     (
//         [](auto, auto) -> ValueType { return {}; },
//         factor,
//         repeat
//         (
//             sequence
//             (
//                 [](auto, auto) -> ValueType { return {}; },
//                 either
//                 (
//                     symbol('-'),
//                     symbol('+')
//                 ),
//                 factor
//             )
//         )
//     )(input);
// }

// auto factor(std::string_view input) -> Parsed
// {
//     return sequence
//     (
//         [](auto, auto) -> ValueType { return {}; },
//         unary,
//         repeat
//         (
//             sequence
//             (
//                 [](auto, auto) -> ValueType { return {}; },
                
//                 either
//                 (
//                     symbol('/'),
//                     symbol('*')
//                 ),
//                 unary
//             )
//         )
//     )(input);
// }

// auto unary(std::string_view input) -> Parsed
// {
//     return either
//     (
//         sequence
//         (
//             [](auto, auto) -> ValueType { return {}; },
//             either
//             (
//                 symbol('!'),
//                 symbol('-')
//             ),
//             unary
//         ),
//         primary
//     )(input);
// }

// auto primary(std::string_view input) -> Parsed
// {
//     return either
//     (
//         str("true"),
//         str("false"),
//         str("nil"),
//         sequence
//         (
//             [](auto, auto, auto) -> std::string { return {}; },
//             symbol('('),
//             expression,
//             symbol(')')
//         )
//     )(input);
// }