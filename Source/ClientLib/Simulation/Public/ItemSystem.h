#pragma once

#include "Item.h"
#include "QueuedEvents.h"

namespace AM
{
namespace Client
{
class World;
class Network;

/**
 * Processes item definition updates.
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

    EventQueue<Item> itemQueue;
};

} // namespace Client
} // namespace AM
