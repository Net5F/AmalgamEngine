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
    return getFileName(filePath).substr(0, filePath.find_last_of('.'));
}

} // End namespace AM
