#pragma once

#include "ItemUpdate.h"
#include "CombineItems.h"
#include "QueuedEvents.h"

namespace AM
{
namespace Client
{
class World;
class Network;

/**
 * Processes item definition updates and item combination updates.
 */
class ItemSystem
{
public:
    ItemSystem(World& inWorld, Network& inNetwork);

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

    /** Used for accessing item data. */
    World& world;
    /** Used for sending requests and receiving item data. */
    Network& network;

    EventQueue<ItemUpdate> itemUpdateQueue;
    EventQueue<CombineItems> combineItemsQueue;
};

} // namespace Client
} // namespace AM
