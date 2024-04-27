#pragma once

#include <string_view>
#include <string>

/**
 * Static functions for working with strings.
 */
namespace AM
{
class StringTools
{
public:
    /**
     * Derives a string ID from a display name by making it all lowercase and
     * replacing spaces with underscores.
     */
    static void deriveStringID(std::string_view displayName, std::string& dest);
};

} // End namespace AM
