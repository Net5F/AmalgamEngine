#pragma once

#include <string>

namespace AM
{
/**
 * Represents an entity's name.
 */
struct Name
{
    /** Used as a "we should never hit this" cap on the length of the name
        string. Only checked in debug builds. */
    static constexpr std::size_t MAX_NAME_LENGTH{50};

    std::string name{""};
};

template<typename S>
void serialize(S& serializer, Name& name)
{
    serializer.text1b(name.name, Name::MAX_NAME_LENGTH);
}

} // End namespace AM
