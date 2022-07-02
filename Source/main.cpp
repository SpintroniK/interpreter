#include "Parser.hpp"

#include <functional>
#include <iostream>
#include <numeric>
#include <string>
#include <string_view>
#include <vector>

using F_t = std::function<Data_t(Data_t, Data_t)>;

template <typename... T, typename TInit, typename FTransform>
TInit reduce_tree(const Tree<T...>& t, TInit init, FTransform t_op)
{
    if(t.second.empty()) // is leaf?
    {
        return std::get<Data_t>(t.first);
    }

    // isn't a leaf
    return std::invoke(std::visit(t_op, t.first), reduce_tree(t.second.front(), init, t_op), reduce_tree(t.second.back(), init, t_op));
}

int main(int argc, char** argv)
{

    const auto args = std::vector(argv, argv + argc);

    for(;;)
    {
        std::cout << "🖉  ";

        std::string line;
        std::getline(std::cin, line);
        const auto input = std::string_view{line};

        if(input == "exit")
        {
            return EXIT_SUCCESS;
        }

        const auto result = expression(input);

        if(result)
        {
            // <int, Plus, Minus, Mult, Div>
            // const auto r = reduce_tree(result->first, Data_t{},
            //                 overloaded                               // Transform "overloaded" function
            //                 {
            //                     [] (Data_t v) -> F_t { return [=](Data_t, Data_t){ return v; }; },
            //                     [] (Plus) -> F_t { return std::plus<int>{}; },
            //                     [] (Minus) -> F_t { return std::minus<int>{}; },
            //                     [] (Mult) -> F_t { return std::multiplies<int>{}; },
            //                     [] (Div) -> F_t { return std::divides<int>{}; },
            //                 });

            std::cout << result->first << std::endl;
        }
        else
        {
            std::cout << "Error, cannot parse input." << std::endl;
        }

    }


    return EXIT_SUCCESS;
}
