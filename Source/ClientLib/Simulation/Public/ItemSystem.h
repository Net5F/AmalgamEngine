#pragma once

#include "ItemUpdate.h"
#include "CombineItems.h"
#include "QueuedEvents.h"

namespace AM
{
namespace Client
{
struct SimulationContext;
class World;
class Network;
class ItemData;

/**
 * Processes item definition updates and item combination updates.
 */
class ItemSystem
{
public:
    ItemSystem(const SimulationContext& inSimContext);
    ~ItemSystem();

    /**
     * Processes item update messages.
     */
    void processItemUpdates();

private:
    /**
     * Loads all items from ItemCache.bin into ItemData.
     */
    void loadItemCache();

    /**
     * Saves all items from ItemData into ItemCache.bin.
     */
    void saveItemCache();

    /** Used for updating inventories. */
    World& world;
    /** Used for sending requests and receiving item data. */
    Network& network;
    /** Used for accessing item data and subscribing to updates. */
    ItemData& itemData;

    EventQueue<ItemUpdate> itemUpdateQueue;
    EventQueue<CombineItems> combineItemsQueue;
};

} // namespace Client
} // namespace AM
