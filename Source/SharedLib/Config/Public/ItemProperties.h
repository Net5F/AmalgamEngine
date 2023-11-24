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
struct ItemDescription
{
    /** The max length of an item's description text. */
    static constexpr std::size_t MAX_TEXT_LENGTH{500};

    std::string text{};
};
template<typename S>
void serialize(S& serializer, ItemDescription& itemDescription)
{
    serializer.text1b(itemDescription.text, ItemDescription::MAX_TEXT_LENGTH);
}

/** The list of properties that may be attached to an item. */
using ItemProperty = std::variant<ItemDescription>;

} // End namespace AM

#endif // defined(AM_OVERRIDE_CONFIG)
