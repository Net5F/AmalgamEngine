#include "InventorySystem.h"
#include "World.h"
#include "Network.h"
#include "ItemData.h"
#include "Inventory.h"
#include "ItemDataRequest.h"
#include "AMAssert.h"
#include "Log.h"
#include <algorithm>

namespace AM
{
namespace Client
{
InventorySystem::InventorySystem(World& inWorld, Network& inNetwork,
                                 const ItemData& inItemData)
: world{inWorld}
, network{inNetwork}
, itemData{inItemData}
, inventoryInitQueue{network.getEventDispatcher()}
, inventoryOperationQueue{network.getEventDispatcher()}
{
}

void InventorySystem::processInventoryUpdates()
{
    // Process any waiting inventory messages.
    InventoryInit inventoryInit{};
    while (inventoryInitQueue.pop(inventoryInit)) {
        initInventory(inventoryInit);
    }

    InventoryOperation inventoryOperation{};
    while (inventoryOperationQueue.pop(inventoryOperation)) {
        std::visit(
            [this](const auto& operation) { processOperation(operation); },
            inventoryOperation.operation);
    }
}

void InventorySystem::initInventory(const InventoryInit& inventoryInit)
{
    // Initialize the player's inventory with the given items.
    world.registry.patch<Inventory>(
        world.playerEntity, [&](Inventory& inventory) {
            // Init the inventory's size.
            inventory.resize(inventoryInit.size);

            // Add all the given items to the player's inventory.
            std::vector<ItemID> itemsToRequest{};
            for (std::size_t i{0}; i < inventoryInit.slots.size(); ++i) {
                const InventoryInit::ItemSlot& itemSlot{inventoryInit.slots[i]};
                inventory.slots[i].ID = itemSlot.ID;
                inventory.slots[i].count = itemSlot.count;

                // If this slot is non-empty and we don't have the latest 
                // definition for the item in it, add it to the vector.
                if (itemSlot.ID
                    && (!(itemData.getItem(itemSlot.ID))
                        || (itemData.getItemVersion(itemSlot.ID)
                            < itemSlot.version))) {
                    itemsToRequest.push_back(itemSlot.ID);
                }
            }

            // Remove duplicates from the vector.
            std::sort(itemsToRequest.begin(), itemsToRequest.end());
            itemsToRequest.erase(
                std::unique(itemsToRequest.begin(), itemsToRequest.end()),
                itemsToRequest.end());

            // Request definitions for any out-of-date items.
            for (ItemID itemID : itemsToRequest) {
                network.serializeAndSend(ItemDataRequest{itemID});
            }
        });
}

void InventorySystem::processOperation(const InventoryAddItem& inventoryAddItem)
{
    // Try to add the item.
    world.registry.patch<Inventory>(
        world.playerEntity, [&](Inventory& inventory) {
            ItemID itemID{inventoryAddItem.itemID};
            if (inventory.addItem(itemID, inventoryAddItem.count,
                                  inventoryAddItem.maxStackSize)) {
                // Successfully added. If we don't have the latest definition
                // for the item, request it.
                ItemVersion itemVersion{inventoryAddItem.version};
                if (!(itemData.getItem(itemID))
                    || (itemData.getItemVersion(itemID) < itemVersion)) {
                    network.serializeAndSend(ItemDataRequest{itemID});
                }
            }
        });
}

void InventorySystem::processOperation(
    const InventoryRemoveItem& inventoryRemoveItem)
{
    // Try to delete the item.
    world.registry.patch<Inventory>(
        world.playerEntity, [&](Inventory& inventory) {
            inventory.removeItem(inventoryRemoveItem.slotIndex,
                                 inventoryRemoveItem.count);
        });
}

void InventorySystem::processOperation(
    const InventoryMoveItem& inventoryMoveItem)
{
    // Try to move the item(s).
    world.registry.patch<Inventory>(
        world.playerEntity, [&](Inventory& inventory) {
            inventory.moveItem(inventoryMoveItem.sourceSlotIndex,
                               inventoryMoveItem.destSlotIndex);
        });
}

} // namespace Client
} // namespace AM
