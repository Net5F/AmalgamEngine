#pragma once

#include "InventoryOperation.h"
#include "EnttObserver.h"
#include "QueuedEvents.h"

namespace AM
{
namespace Server
{
class World;
class Network;
class ItemData;
class ISimulationExtension;

/**
 * Processes inventory update messages and requests needed item definitions.
 */
class InventorySystem
{
public:
    InventorySystem(World& inWorld, Network& inNetwork,
                    const ItemData& itemData);

    /**
     * Sends initial inventory state to newly-logged-on clients.
     */
    void sendInventoryInits();

    /**
     * Processes inventory manipulation requests and sends inventory data.
     */
    void processInventoryUpdates();

    void setExtension(ISimulationExtension* inExtension);

private:
    /**
     * If the request is valid, adds the item to the specified inventory.
     */
    void processOperation(NetworkID clientID,
                          const InventoryAddItem& inventoryAddItem);

    /**
     * Removes the specified item from the requester's inventory.
     */
    void processOperation(NetworkID clientID,
                          const InventoryRemoveItem& inventoryRemoveItem);

    /**
     * Moves the specified item to the specified slot in the requestor's
     * inventory.
     */
    void processOperation(NetworkID clientID,
                          const InventoryMoveItem& inventoryMoveItem);

    /** Used for accessing inventory data. */
    World& world;
    /** Used for receiving requests and sending item and inventory data. */
    Network& network;
    /** Used for accessing item data. */
    const ItemData& itemData;
    /** If non-nullptr, contains the project's simulation extension functions.
        Used for checking if item change requests are valid. */
    ISimulationExtension* extension;

    /** Observes player Inventory component creation so we can send the initial
        inventory state to a newly-logged-on player. */
    EnttObserver playerInventoryObserver;

    EventQueue<InventoryOperation> inventoryOperationQueue;
};

} // namespace Server
} // namespace AM
