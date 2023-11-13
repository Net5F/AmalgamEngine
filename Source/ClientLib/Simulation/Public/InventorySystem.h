#pragma once

#include "InventoryInit.h"
#include "InventoryAddItem.h"
#include "InventoryDeleteItem.h"
#include "InventoryMoveItem.h"
#include "QueuedEvents.h"

namespace AM
{
namespace Client
{
class World;
class Network;

/**
 * Processes inventory updates and requests needed item definitions.
 */
class InventorySystem
{
public:
    InventorySystem(World& inWorld, Network& inNetwork);

    /**
     * Processes inventory update messages.
     */
    void processInventoryUpdates();

private:
    /**
     * Sets the player's inventory to the given state.
     */
    void initInventory(const InventoryInit& inventoryInit);

    /**
     * Adds the item to the player's inventory.
     */
    void addItem(const InventoryAddItem& inventoryAddItem);

    /**
     * Deletes the specified item from the player's inventory.
     */
    void deleteItem(const InventoryDeleteItem& inventoryDeleteItem);

    /**
     * Moves the specified item to the specified slot in the player's 
     * inventory.
     */
    void moveItem(const InventoryMoveItem& inventoryMoveItem);

    /** Used for accessing item and inventory data. */
    World& world;
    /** Used for sending requests and receiving inventory data. */
    Network& network;

    EventQueue<InventoryInit> inventoryInitQueue;
    EventQueue<InventoryAddItem> inventoryAddItemQueue;
    EventQueue<InventoryDeleteItem> inventoryDeleteItemQueue;
    EventQueue<InventoryMoveItem> inventoryMoveItemQueue;
};

} // namespace Client
} // namespace AM
