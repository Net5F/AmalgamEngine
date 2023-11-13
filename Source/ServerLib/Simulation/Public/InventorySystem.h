#pragma once

#include "InventoryAddItem.h"
#include "InventoryDeleteItem.h"
#include "InventoryMoveItem.h"
#include "QueuedEvents.h"
#include "entt/entity/observer.hpp"
#include "entt/entity/registry.hpp"

namespace AM
{
namespace Server
{
class World;
class Network;
class ISimulationExtension;

/**
 * Processes inventory updates and requests needed item definitions.
 */
class InventorySystem
{
public:
    InventorySystem(World& inWorld, Network& inNetwork);

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
    void addItem(const InventoryAddItem& inventoryAddItem);

    /**
     * Deletes the specified item from the requestor's inventory.
     */
    void deleteItem(const InventoryDeleteItem& inventoryDeleteItem);

    /**
     * Moves the specified item to the specified slot in the requestor's 
     * inventory.
     */
    void moveItem(const InventoryMoveItem& inventoryMoveItem);

    /** Used for accessing item and inventory data. */
    World& world;
    /** Used for receiving requests and sending item and inventory data. */
    Network& network;
    /** If non-nullptr, contains the project's simulation extension functions.
        Used for checking if item change requests are valid. */
    ISimulationExtension* extension;

    /** Observes player Inventory component creation so we can send the initial 
        inventory state to a newly-logged-on player. */
    entt::observer playerInventoryObserver;

    EventQueue<InventoryAddItem> inventoryAddItemQueue;
    EventQueue<InventoryDeleteItem> inventoryDeleteItemQueue;
    EventQueue<InventoryMoveItem> inventoryMoveItemQueue;
};

} // namespace Server
} // namespace AM
