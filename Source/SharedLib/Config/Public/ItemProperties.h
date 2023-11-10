#pragma once

// Use the project's ItemProperties, if one is provided.
#if defined(AM_OVERRIDE_DEFAULT_CONFIGS)
#include "Override/ItemProperties.h"
#else
#include <SDL_stdinc.h>
#include <string>
#include <variant>

namespace AM
{

/** The item's description. */
struct Description
{
    std::string text{};
};
template<typename S>
void serialize(S& serializer, Description& description)
{
    serializer.text1b(description.text);
}

/** The list of properties that may be attached to an item. */
using ItemProperty = std::variant<Description>;

} // End namespace AM

#endif // defined(AM_OVERRIDE_CONFIG)
