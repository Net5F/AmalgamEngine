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

    entt::sigh<void(const Item&)> itemUpdatedSig;

public:
    /** An item's definition has been updated. */
    entt::sink<entt::sigh<void(const Item&)>> itemUpdated;
};

} // namespace Client
} // namespace AM
