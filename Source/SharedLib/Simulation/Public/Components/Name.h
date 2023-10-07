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
    static constexpr std::size_t MAX_LENGTH{50};

    std::string value{""};
};

template<typename S>
void serialize(S& serializer, Name& name)
{
    serializer.text1b(name.value, Name::MAX_LENGTH);
}

} // End namespace AM
