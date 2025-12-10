#pragma once

#include "ItemID.h"
#include "EntityItemHandlerScript.h"
#include <vector>
#include <string_view>

namespace AM
{
namespace Server
{
/**
 * An entity's handling logic for when an item is used on it.
 *
 * A single handler script is used for all items. Within it, you can check for 
 * specific item IDs to handle them individually, or check for the presence of 
 * specific item properties to handle them as groups.
 *
 * There's no default handler. You must add a handler in order for anything to 
 * happen when an item is used on this entity.
 */
struct ItemHandler {
    /** The handler script to run. */
    EntityItemHandlerScript handlerScript{};
};

template<typename S>
void serialize(S& serializer, ItemHandler& itemHandler)
{
    serializer.object(itemHandler.handlerScript);
}

} // namespace Server
} // namespace AM
