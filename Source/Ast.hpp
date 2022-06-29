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

template <typename... T>
struct Tree : std::pair<std::variant<T...>, std::vector<Tree<T...>>>
{
};