#pragma once

#include <string>

namespace AM
{
/**
 * Represents an entity's name.
 *
 * All entities have a name.
 */
struct Name
{
    /** The max length of a name. */
    static constexpr std::size_t MAX_LENGTH{50};

    std::string value{""};
};

template<typename S>
void serialize(S& serializer, Name& name)
{
    serializer.text1b(name.value, Name::MAX_LENGTH);
}

} // End namespace AM
