#pragma once

namespace AM
{

/**
 * Common helper functions for constexpr use.
 *
 * Note: This shouldn't be necessary since a lot of std algorithms are 
 *       constexpr in gcc, but they aren't constexpr in msvc yet.
 */
class ConstexprTools
{
public:
    /**
     * Computes the smallest integer value not less than the given value.
     *
     * Reference: https://stackoverflow.com/a/66146159/4258629
     */
    static constexpr int ceilInt(float value)
    {
        const int truncated{static_cast<int>(value)};
        return (value > truncated) ? (truncated + 1) : truncated;
    }

    /**
     * Computes the largest integer value not greater than the given value.
     *
     * Reference: https://stackoverflow.com/a/66146159/4258629
     */
    static constexpr int floorInt(float value)
    {
        const int truncated{static_cast<int>(value)};
        return (value < truncated) ? (truncated - 1) : truncated;
    }
};

} // End namespace AM
