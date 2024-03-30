#pragma once

#include "EngineMessageType.h"
#include "ItemID.h"
#include "NetworkDefs.h"
#include "entt/fwd.hpp"
#include "entt/entity/entity.hpp"
#include <SDL_stdinc.h>

namespace AM
{
/**
 * Sent by a client to request that an item be added to an inventory, or by the
 * server to tell a client that an item was added.
 */
struct InventoryAddItem {
    /** The entity to add the item to.
        When sent by the server, this can be ignored and assumed to be the
        client entity (clients are only sent updates for their own inventory).
     */
    entt::entity entity{entt::null};

    /** The item to add. */
    ItemID itemID{NULL_ITEM_ID};

    /** How many of the item to add. */
    Uint8 count{0};

    /** How large a stack of this item can be.
        This value is available in the item's definition, but we have to send 
        it here because the client may not yet have the definition.
        When sent by the client, this field is ignored. */
    Uint8 maxStackSize{1};

    /** The item's version number. Used by the client to tell if it
        already has the latest definition for this item, or if it needs
        to request it.
        When sent by the client, this field is ignored. */
    ItemVersion version{0};
};

template<typename S>
void serialize(S& serializer, InventoryAddItem& inventoryAddItem)
{
    serializer.value4b(inventoryAddItem.entity);
    serializer.value2b(inventoryAddItem.itemID);
    serializer.value1b(inventoryAddItem.count);
    serializer.value1b(inventoryAddItem.maxStackSize);
    serializer.value2b(inventoryAddItem.version);
}

} // namespace AM
