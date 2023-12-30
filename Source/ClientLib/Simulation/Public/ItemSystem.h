#pragma once

#include "ItemUpdate.h"
#include "CombineItems.h"
#include "QueuedEvents.h"
#include "entt/signal/sigh.hpp"

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
    /** Used for accessing item data. */
    World& world;
    /** Used for sending requests and receiving item data. */
    Network& network;

    EventQueue<ItemUpdate> itemUpdateQueue;
    EventQueue<CombineItems> combineItemsQueue;

    entt::sigh<void(const Item&)> itemUpdateSig;

public:
    /** We've received the latest definition for an item.
        This may mean that an item was actually updated, or we may have just
        requested the latest data to see if it was updated. */
    entt::sink<entt::sigh<void(const Item&)>> itemUpdate;
};

} // namespace Client
} // namespace AM
