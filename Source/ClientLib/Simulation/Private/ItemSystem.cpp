#include "ItemSystem.h"
#include "World.h"
#include "Network.h"
#include "Inventory.h"
#include "ItemDataRequest.h"
#include "Log.h"

namespace AM
{
namespace Client
{
ItemSystem::ItemSystem(World& inWorld, Network& inNetwork)
: world{inWorld}
, network{inNetwork}
, itemUpdateQueue{network.getEventDispatcher()}
, combineItemsQueue{network.getEventDispatcher()}
, itemUpdateSig{}
, itemUpdate{itemUpdateSig}
{
}

void ItemSystem::processItemUpdates()
{
    ItemData& itemData{world.itemData};

    // Process any waiting item definition updates.
    ItemUpdate itemUpdate{};
    while (itemUpdateQueue.pop(itemUpdate)) {
        // If the item exists, update it (even if there are no changes, it
        // doesn't hurt to update it).
        Item item{itemUpdate.displayName, "", itemUpdate.itemID,
                  itemUpdate.iconID, itemUpdate.supportedInteractions};
        const Item* newItem{nullptr};
        if (itemData.itemExists(itemUpdate.itemID)) {
            newItem = itemData.updateItem(item);
        }
        else {
            // Item doesn't exist, create it.
            newItem = itemData.createItem(item);
        }

        // Signal that we received an item update.
        itemUpdateSig.publish(*newItem);
    }

    // Process any waiting item combinations.
    CombineItems combineItems{};
    while (combineItemsQueue.pop(combineItems)) {
        world.registry.patch<Inventory>(
            world.playerEntity, [&](Inventory& inventory) {
                inventory.combineItems(combineItems.sourceSlotIndex,
                                       combineItems.targetSlotIndex,
                                       combineItems.resultItemID);
            });

        // If we don't have the latest definition for the new item, request
        // it.
        ItemID resultItemID{combineItems.resultItemID};
        if (!(world.itemData.itemExists(resultItemID))
            || (world.itemData.getItemVersion(resultItemID)
                < combineItems.resultItemVersion)) {
            network.serializeAndSend(ItemDataRequest{resultItemID});
        }
    }
}

} // namespace Client
} // namespace AM
