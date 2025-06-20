#pragma once

#include <string>

namespace AM
{
/**
 * Represents an entity's name.
 *
 * All client/server-synchronized entities have a name. Client-only A/V 
 * entities don't have names.
 */
struct Name {
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
