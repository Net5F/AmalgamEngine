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
     *
     * Note: This uses an out param instead of returning the string because it's
     *       used in performance-sensitive situations where we need to re-use 
     *       a pre-allocated string.
     */
    static void deriveStringID(std::string_view displayName, std::string& dest);

    /**
     * Returns the name of the file at the given path, including extension.
     */
    static std::string_view getFileName(std::string_view filePath);

    /**
     * Returns the name of the file at the given path, with no extension.
     */
    static std::string_view getFileNameNoExtension(std::string_view filePath);

    /**
     * Returns true if pathA starts with the characters in pathB, ignoring 
     * differences in slash type used ('/' vs '\').
     */
    static bool pathStartsWith(std::string_view pathA, std::string_view pathB);
};

} // End namespace AM
