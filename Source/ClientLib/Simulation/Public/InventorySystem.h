#pragma once

#include "InventoryInit.h"
#include "InventoryOperation.h"
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
    void processOperation(const InventoryAddItem& inventoryAddItem);

    /**
     * Removes the specified item from the player's inventory.
     */
    void processOperation(const InventoryRemoveItem& inventoryRemoveItem);

    /**
     * Moves the specified item to the specified slot in the player's
     * inventory.
     */
    void processOperation(const InventoryMoveItem& inventoryMoveItem);

    /** Used for accessing item and inventory data. */
    World& world;
    /** Used for sending requests and receiving inventory data. */
    Network& network;

    EventQueue<InventoryInit> inventoryInitQueue;
    EventQueue<InventoryOperation> inventoryOperationQueue;
};

} // namespace Client
} // namespace AM
