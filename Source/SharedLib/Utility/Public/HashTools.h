#pragma once

#include <functional>
#include <string_view>
#include <string>

/**
 * This file contains helper functions for hashing custom types.
 */
namespace AM
{

/**
 * Hashes the given value and adds it to the given seed.
 *
 * Based on boost::hash_combine().
 *
 * Reference:
 * https://stackoverflow.com/questions/37007307/fast-hash-function-for-stdvector
 */
template<class T>
inline void hash_combine(std::size_t& seed, T const& value)
{
    seed ^= std::hash<T>()(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

/**
 * A hashing functor that we use to enable heterogenous access, e.g.:
 *   std::unordered_map<std::string, int, string_hash, std::equal_to> map;
 *   map.find(std::string_view{"Hi"});
 * (notice that the key type is std::string, but we pass in a std::string_view).
 *
 * Reference: https://www.cppstories.com/2021/heterogeneous-access-cpp20/
 */
struct string_hash {
    using is_transparent = void;
    [[nodiscard]] size_t operator()(const char* txt) const
    {
        return std::hash<std::string_view>{}(txt);
    }
    [[nodiscard]] size_t operator()(std::string_view txt) const
    {
        return std::hash<std::string_view>{}(txt);
    }
    [[nodiscard]] size_t operator()(const std::string& txt) const
    {
        return std::hash<std::string>{}(txt);
    }
};

} // End namespace AM
