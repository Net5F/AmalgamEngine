#include "ItemSystem.h"
#include "World.h"
#include "Network.h"
#include "Inventory.h"
#include "ItemUpdateRequest.h"
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
, itemUpdatedSig{}
, itemUpdated{itemUpdatedSig}
{
}

void ItemSystem::processItemUpdates()
{
    ItemData& itemData{world.itemData};

    // Process any waiting item definition updates.
    ItemUpdate itemUpdate{};
    while (itemUpdateQueue.pop(itemUpdate)) {
        // If the item exists, update it.
        Item item{itemUpdate.displayName, "", itemUpdate.numericID,
                  itemUpdate.supportedInteractions};
        if (itemData.itemExists(itemUpdate.numericID)) {
            itemData.updateItem(item);
        }
        else {
            // Item doesn't exist, create it.
            itemData.createItem(item);
        }
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
            network.serializeAndSend(ItemUpdateRequest{resultItemID});
        }
    }
}

} // namespace Client
} // namespace AM
