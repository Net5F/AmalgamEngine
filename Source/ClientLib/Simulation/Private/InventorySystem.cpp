#include "InventorySystem.h"
#include "World.h"
#include "Network.h"
#include "Inventory.h"
#include "ItemRequest.h"
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
    // Note: All clients have inventories so we don't need to check for it.
    Inventory& inventory{world.registry.get<Inventory>(world.playerEntity)};
    AM_ASSERT(inventory.items.size() == 0,
              "Inventory should be empty when we receive init.");

    // Add all the given items to the player's inventory.
    for (const InventoryInit::ItemSlot& itemSlot : inventoryInit.items) {
        inventory.items.emplace_back(itemSlot.ID, itemSlot.count);

        // If we don't have the latest definition for an item, request it.
        if (!(world.itemData.itemExists(itemSlot.ID))
            || (world.itemData.getItemVersion(itemSlot.ID)
                < itemSlot.version)) {
            network.serializeAndSend(ItemRequest{itemSlot.ID});
        }
    }
}

void InventorySystem::addItem(const InventoryAddItem& inventoryAddItem)
{
    // Try to add the item.
    // Note: All clients have inventories so we don't need to check for it.
    Inventory& inventory{world.registry.get<Inventory>(world.playerEntity)};
    inventory.addItem(inventoryAddItem.itemID, inventoryAddItem.count);
}

void InventorySystem::deleteItem(const InventoryDeleteItem& inventoryDeleteItem)
{
    // Try to delete the item.
    // Note: All clients have inventories so we don't need to check for it.
    Inventory& inventory{world.registry.get<Inventory>(world.playerEntity)};
    inventory.deleteItem(inventoryDeleteItem.slotIndex,
                         inventoryDeleteItem.count);
}

void InventorySystem::moveItem(const InventoryMoveItem& inventoryMoveItem)
{
    // Try to move the item(s).
    // Note: All clients have inventories so we don't need to check for it.
    Inventory& inventory{world.registry.get<Inventory>(world.playerEntity)};
    inventory.moveItem(inventoryMoveItem.sourceSlotIndex,
                       inventoryMoveItem.destSlotIndex);
}

} // namespace Client
} // namespace AM
