#pragma once

#include <functional>

/**
 * This file contains helper functions for hashing custom types.
 *
 * Reference:
 * https://stackoverflow.com/questions/37007307/fast-hash-function-for-stdvector
 */
namespace AM
{

/**
 * Hashes the given value and adds it to the given seed.
 *
 * Based on boost::hash_combine().
 */
template<class T>
inline void hash_combine(std::size_t& seed, T const& value)
{
    seed ^= std::hash<T>()(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

} // End namespace AM
