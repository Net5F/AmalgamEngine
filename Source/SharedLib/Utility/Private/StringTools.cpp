#include "StringTools.h"
#include <algorithm>

namespace AM
{

void StringTools::deriveStringID(std::string_view displayName,
                                 std::string& dest)
{
    // Derive a string ID from the given display name.
    dest.resize(displayName.size());
    std::transform(displayName.begin(), displayName.end(), dest.begin(),
                   [](unsigned char c) -> unsigned char {
                       // Replace spaces with underscores.
                       if (c == ' ') {
                           return '_';
                       }
                       // Make the string all lowercase.
                       return std::tolower(c);
                   });
}

std::string_view StringTools::getFileName(std::string_view filePath)
{
    return filePath.substr(filePath.find_last_of("/\\") + 1);
}

std::string_view StringTools::getFileNameNoExtension(std::string_view filePath)
{
    std::string_view fileName{getFileName(filePath)};
    return fileName.substr(0, fileName.find_last_of('.'));
}

bool StringTools::pathStartsWith(std::string_view pathA, std::string_view pathB)
{
    // If pathB is longer than pathA, pathA can't possibly start with it.
    if (pathB.size() > pathA.size()) {
        return false;
    }

    // Compare char-by-char.
    for (int index{0}; index < pathB.size(); ++index) {
        char charA{pathA.at(index)};
        char charB{pathB.at(index)};

        // Normalize slashes to '/';
        if (charA == '\\') {
            charA = '/';
        }
        if (charB == '\\') {
            charB = '/';
        }

        if (charA != charB) {
            return false;
        }
    }

    return true;
}

} // End namespace AM
