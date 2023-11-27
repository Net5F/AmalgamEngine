#include "InventorySystem.h"
#include "World.h"
#include "Network.h"
#include "Inventory.h"
#include "ItemUpdateRequest.h"
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
, inventoryAddItemQueue{network.getEventDispatcher()}
, inventoryDeleteItemQueue{network.getEventDispatcher()}
, inventoryMoveItemQueue{network.getEventDispatcher()}
{
}

void InventorySystem::processInventoryUpdates()
{
    // Process any waiting inventory messages.
    InventoryInit inventoryInit{};
    while (inventoryInitQueue.pop(inventoryInit)) {
        initInventory(inventoryInit);
    }

    InventoryAddItem inventoryAddItem{};
    while (inventoryAddItemQueue.pop(inventoryAddItem)) {
        addItem(inventoryAddItem);
    }

    InventoryDeleteItem inventoryDeleteItem{};
    while (inventoryDeleteItemQueue.pop(inventoryDeleteItem)) {
        deleteItem(inventoryDeleteItem);
    }

    InventoryMoveItem inventoryMoveItem{};
    while (inventoryMoveItemQueue.pop(inventoryMoveItem)) {
        moveItem(inventoryMoveItem);
    }
}

void InventorySystem::initInventory(const InventoryInit& inventoryInit)
{
    // Initialize the player's inventory with the given items.
    world.registry.patch<Inventory>(
        world.playerEntity, [&](Inventory& inventory) {
            AM_ASSERT(inventory.items.size() == 0,
                      "Inventory should be empty when we receive init.");

            // Add all the given items to the player's inventory.
            std::vector<ItemID> itemsToRequest{};
            for (const InventoryInit::ItemSlot& itemSlot :
                 inventoryInit.items) {
                inventory.items.emplace_back(itemSlot.ID, itemSlot.count);

                // If we don't have the latest definition for an item, request
                // it.
                if (!(world.itemData.itemExists(itemSlot.ID))
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

            for (ItemID itemID : itemsToRequest) {
                network.serializeAndSend(ItemUpdateRequest{itemID});
            }
        });
}

void InventorySystem::addItem(const InventoryAddItem& inventoryAddItem)
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
                    network.serializeAndSend(ItemUpdateRequest{itemID});
                }
            }
        });
}

void InventorySystem::deleteItem(const InventoryDeleteItem& inventoryDeleteItem)
{
    // Try to delete the item.
    world.registry.patch<Inventory>(
        world.playerEntity, [&](Inventory& inventory) {
            inventory.deleteItem(inventoryDeleteItem.slotIndex,
                                 inventoryDeleteItem.count);
        });
}

void InventorySystem::moveItem(const InventoryMoveItem& inventoryMoveItem)
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
