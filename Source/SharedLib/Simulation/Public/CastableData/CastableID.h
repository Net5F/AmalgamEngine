#pragma once

#include "ItemInteractionType.h"
#include "EntityInteractionType.h"
#include "SpellType.h"
#include "bitsery/ext/std_variant.h"
#include <variant>

namespace AM
{

/**
 * Identifies a castable.
 *
 * Castables fall into one of 3 types (item interaction, entity interaction, 
 * spell), and are associated with an enum value. Only one castable can be 
 * assigned to each type+value, so this ID is unique.
 */
using CastableID
    = std::variant<ItemInteractionType, EntityInteractionType, SpellType>;

template<typename S>
void serialize(S& serializer, CastableID& castableID)
{
    serializer.ext(castableID,
                   bitsery::ext::StdVariant{[](S& serializer, auto& type) {
                       serializer.value1b(type);
                   }});
}

} // namespace AM
