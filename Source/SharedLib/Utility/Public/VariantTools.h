#pragma once

namespace AM
{

/**
 * Common helper functions for std::variant use.
 */
namespace VariantTools
{

template<typename... Ts>
struct Overload : Ts... {
    using Ts::operator()...;
};

template<class... Ts>
Overload(Ts...) -> Overload<Ts...>;

}

} // End namespace AM
