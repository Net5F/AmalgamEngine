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

} // End namespace AM
