#include "InventorySystem.h"
#include "World.h"
#include "Network.h"
#include "Inventory.h"
#include "ItemDataRequest.h"
#include "AMAssert.h"
#include "Log.h"

namespace AM
{
namespace Client
{
InventorySystem::InventorySystem(World& inWorld, Network& inNetwork)
: world{inWorld}
, network{inNetwork}
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
            for (const InventoryInit::ItemSlot& itemSlot :
                 inventoryInit.slots) {
                inventory.slots.emplace_back(itemSlot.ID, itemSlot.count);

                // If we don't have the latest definition for an item, add it
                // to the vector.
                if ((itemSlot.ID && !(world.itemData.itemExists(itemSlot.ID)))
                    || (world.itemData.getItemVersion(itemSlot.ID)
                        < itemSlot.version)) {
                    // Only request each ID once.
                    if (std::find(itemsToRequest.begin(), itemsToRequest.end(),
                                  itemSlot.ID)
                        == itemsToRequest.end()) {
                        itemsToRequest.push_back(itemSlot.ID);
                    }
                }
            }

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
            if (inventory.addItem(itemID, inventoryAddItem.count)) {
                // Successfully added. If we don't have the latest definition 
                // for the item, request it.
                ItemVersion itemVersion{inventoryAddItem.version};
                if (!(world.itemData.itemExists(itemID))
                    || world.itemData.getItemVersion(itemID) < itemVersion) {
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
