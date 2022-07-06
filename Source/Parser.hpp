#pragma once

#include "Ast.hpp"

#include <cctype>
#include <concepts>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

// Basic definitions

template <typename T>
using Parsed_t = std::optional<std::pair<T, std::string_view>>;

template <typename P>
concept Parser =
    std::regular_invocable<P, std::string_view> &&
    requires (std::invoke_result_t<P, std::string_view> result) {
        std::same_as<
            decltype(result),
            Parsed_t<typename decltype(result)::value_type::first_type>>;
    };

template <Parser P>
using Parser_result_t = std::invoke_result_t<P, std::string_view>;

template <Parser P>
using Parser_value_t = typename Parser_result_t<P>::value_type::first_type;

template <typename F, typename... Args>
concept Parser_combinator =
    std::regular_invocable<F, Args...> &&
    Parser<std::invoke_result_t<F, Args...>>;

template <typename F, typename... Args>
requires Parser_combinator<F, Args...>
using Parser_combinator_value_t = std::invoke_result_t<F, Args...>;

inline constexpr auto
item = [](std::string_view input) -> Parsed_t<char>
{
    if (input.empty()) {
        return {};
    } else {
        return {{input[0], input.substr(1)}};
    }
};

// Sequencing parsers

template <typename T>
constexpr Parser auto
unit(T const& thing)
{
    return [thing](std::string_view input) -> Parsed_t<T>
    {
        return {{thing, input}};
    };
}

template <Parser P, Parser_combinator<Parser_value_t<P>> F>
constexpr Parser auto
operator&(P parser, F func)
{
    using Parser_t = Parser_combinator_value_t<F, Parser_value_t<P>>;
    return [=](std::string_view input) -> Parser_result_t<Parser_t>
    {
        if (auto const& result = std::invoke(parser, input)) {
            return std::invoke(std::invoke(func, result->first), result->second);
        } else {
            return {};
        }
    };
}

constexpr Parser auto
chain(Parser auto parser, auto... funcs)
{
    if constexpr (std::is_pointer_v<decltype(parser)>) {
        return ([parser](auto input){ return std::invoke(parser, input); } & ... & funcs);
    } else {
        return (parser & ... & funcs);
    }
}

static_assert(
    chain(unit('y'), [](auto){ return item; })("x") == item("x"),
    "unit<T> must be a left identity element for chain"
);
static_assert(
    chain(item, unit<char>)("x") == item("x"),
    "unit<T> must be a right identity element for chain"
);
static_assert(
    chain(chain(item, [](auto){ return unit(item); }), [](auto){ return unit(item); })("xyz") ==
    chain(item, [](auto){ return chain(unit(item), [](auto){ return unit(item); }); })("xyz"),
    "chain must be associative"
);

inline constexpr auto papply = []<typename F, typename... Args>(F&& f, Args&&... args)
{
    if constexpr (std::invocable<F, Args...>) {
        return std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
    } else {
        return std::bind_front(std::forward<F>(f), std::forward<Args>(args)...);
    }
};

template <Parser P, Parser Q>
constexpr Parser auto
operator^(P p, Q q)
{
    using Result_t = std::invoke_result_t<decltype(papply), Parser_value_t<P>, Parser_value_t<Q>>;
    return [=](std::string_view input) -> Parsed_t<Result_t>
    {
        if (auto const& pr = std::invoke(p, input)) {
            if (auto const& qr = std::invoke(q, pr->second)) {
                return {{papply(pr->first, qr->first), qr->second}};
            } else {
                return {};
            }
        } else {
            return {};
        }
    };
}

template <typename F, Parser... Ps>
requires std::regular_invocable<F, Parser_value_t<Ps>...>
constexpr Parser auto
sequence(F func, Ps... parsers)
{
    return (unit(func) ^ ... ^ parsers);
}

template <typename T, Parser P, std::regular_invocable<T, Parser_value_t<P>> F>
requires std::convertible_to<std::invoke_result_t<F, T, Parser_value_t<P>>, T>
class reduce_many
{
    T init; P parser; F func;

public:

    constexpr
    reduce_many(T const& thing, P const& p, F const& fn)
        : init{thing}, parser{p}, func{fn}
    {}

    constexpr auto
    operator()(std::string_view input) const -> Parsed_t<T>
    {
        return either(
            chain(
                parser,
                [this](auto const& thing)
                {
                    return reduce_many{std::invoke(func, init, thing), parser, func};
                }
            ),
            unit(init)
        )(input);
    }
};

template <Parser P>
requires std::same_as<Parser_value_t<P>, char>
constexpr Parser auto
many(P parser)
{
    return reduce_many(
        std::string{},
        parser,
        [](std::string const& st, char ch){ return st + ch; }
    );
}

template <Parser P>
requires std::same_as<Parser_value_t<P>, char>
constexpr Parser auto
some(P parser)
{
    return sequence(
        [](char ch, std::string const& st){ return std::string(1, ch) + st; },
        parser,
        many(parser)
    );
}

// Making eithers

template <typename T>
inline constexpr auto
empty = [](std::string_view) -> Parsed_t<T>
{
    return {};
};

template <Parser P, Parser Q>
requires std::convertible_to<Parser_value_t<P>, Parser_value_t<Q>>
constexpr Parser auto
operator|(P p, Q q)
{
    return [=](std::string_view input) -> Parser_result_t<Q>
    {
        if (auto const& result = std::invoke(p, input)) {
            return result;
        } else {
            return std::invoke(q, input);
        }
    };
}

constexpr Parser auto
either(Parser auto parser, Parser auto... parsers)
{
    if constexpr (std::is_pointer_v<decltype(parser)>) {
        return ([parser](auto input){ return std::invoke(parser, input); } | ... | parsers);
    } else {
        return (parser | ... | parsers);
    }
}

static_assert(
    either(empty<char>, item)("x") == item("x"),
    "empty<T> must be an identity element for either"
);

template <Parser P, Parser Q>
constexpr Parser auto
pass_empty(P p, Q q)
{
    return [=](std::string_view input) -> Parser_result_t<Q>
    {
        if (auto const& result = std::invoke(p, input)) {
            return {};
        } else {
            return std::invoke(q, input);
        }
    };
}

// Derived primitives

constexpr Parser auto
skip(Parser auto p, Parser auto q)
{
    return either(chain(p, [q](auto){ return q; }), q);
}

template <typename Pr, Parser P = decltype(item)>
requires std::predicate<Pr, Parser_value_t<P>>
constexpr Parser auto
satisfy(Pr pred, P parser = item)
{
    return chain(
        parser,
        [pred](auto const& th) -> Parser auto
        {
            return [pred, th](std::string_view input) -> Parsed_t<Parser_value_t<P>>
            {
                if (std::invoke(pred, th)) return {{th, input}}; else return {};
            };
        }
    );
}

template <Parser P>
constexpr Parser auto
maybe(P parser)
{
    return [parser](std::string_view input) -> Parsed_t<std::optional<Parser_value_t<P>>>
    {
        return either(
            chain(parser, [](auto const& thing){ return unit(std::optional{thing}); }),
            unit(std::optional<Parser_value_t<P>>{})
        )(input);
    };
}

constexpr Parser auto
digit = satisfy([](char x){ return x >= '0' && x <= '9'; });

constexpr Parser auto
lower = satisfy([](char x){ return x >= 'a' && x <= 'z'; });

constexpr Parser auto
upper = satisfy([](char x){ return x >= 'A' && x <= 'Z'; });

constexpr Parser auto
letter = either(lower, upper);

constexpr Parser auto
alphanum = either(letter, digit);

constexpr Parser auto
symbol(char x)
{
    return satisfy([x](char y){ return x == y; });
}

constexpr Parser auto
str(std::string_view match)
{
    return [match](std::string_view input) -> Parsed_t<std::string>
    {
        if (input.starts_with(match)) 
        {
            return {{ std::string{match}, {input.begin() + match.size(), input.end()} }};
        } 

        return {};
    };
}
static_assert(Parser_combinator<decltype(str), std::string_view>);

//  Handling spacing

inline constexpr Parser auto space = satisfy(::isspace);

inline Parser auto whitespace = many(space);

inline Parser auto eol = either(str("\r\n"), str("\r"), str("\n"));

inline Parser auto separator = either(eol, sequence([](auto ch){ return std::string(1, ch); }, space));

constexpr Parser auto
token(Parser auto parser)
{
    return sequence(
        [](auto const& thing, auto){ return thing; },
        skip(whitespace, parser),
        whitespace
    );
}


template <typename T>
auto appended_vector(std::vector<T> x, T const& y) -> std::vector<T>
{
    x.push_back(y);
    return x;
}

template <Parser P>
Parser auto repeat(P parser)
{
    using T = Parser_value_t<P>;
    using Ts = std::vector<T>;
    return reduce_many(
        Ts{},
        parser,
        [](Ts const& ts, T const& t){ return appended_vector(ts, t); }
    );
}

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

template <typename E>
auto MakeExpr(auto... e)
{
    return Expr{E{std::make_shared<Expr>(e)...}};
}

using Data_t = double;
using Variant_t = std::variant<Data_t, std::string, Add, Sub, Mul, Div, Neg>;

struct Expr : Variant_t {};

using Value_t = Expr;
using Parsed = Parsed_t<Value_t>;

using Node = Value_t;

// template <typename T>
// Node MakeNode(const T& n, const std::vector<Node>& c = {})
// {
//     return Node{ std::make_pair(n, c) };
// }

Parser auto natural = chain
(
    some(digit),
    [](const std::string& digits) { return unit(std::stod(digits)); }
);


// Parser auto vnatural = chain
// (
//     some(digit),
//     [](const std::string& digits) { return unit(MakeNode(std::stoi(digits))); }
// );

Parser auto enatural = chain
(
    some(digit),
    [](const std::string& digits) { return unit(Expr{std::stod(digits)}); }
);

Parser auto integer = either
(
    natural,
    chain
    (
        symbol('-'),
        [](auto) { return natural; },
        [](const auto& nat) { return unit(-nat); }
    )
);

// Parser auto vinteger = either
// (
//     vnatural,
//     chain
//     (
//         symbol('-'),
//         [](auto) { return natural; },
//         [](const auto& nat) { return unit(MakeNode(-nat)); }
//     )
// );

Parser auto einteger = either
(
    enatural,
    chain
    (
        symbol('-'),
        [](auto) { return natural; },
        [](const auto& nat) { return unit(Expr{-nat}); }
    )
);

// constexpr Parser auto
// vstr(std::string_view match)
// {
//     return [match](std::string_view input) -> Parsed
//     {
//         if (input.starts_with(match)) 
//         {
//             return {{ MakeNode(std::string{match}), {input.begin() + match.size(), input.end()} }};
//         }

//         return {};
//     };
// }

constexpr Parser auto
estr(std::string_view match)
{
    return [match](std::string_view input) -> Parsed
    {
        if(input.starts_with(match))
        {
            return {{ Expr{std::string{match}}, {input.begin() + match.size(), input.end()} }};
        }

        return {};
    };
}

auto expression(std::string_view) -> Parsed;
auto term(std::string_view) -> Parsed;
auto factor(std::string_view) -> Parsed;
auto unary(std::string_view) -> Parsed;
auto primary(std::string_view) -> Parsed;
auto real(std::string_view) -> Parsed;