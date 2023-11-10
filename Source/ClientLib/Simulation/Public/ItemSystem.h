#pragma once

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
class ItemSystem
{
public:
    ItemSystem(World& inWorld, Network& inNetwork);

    /**
     * Processes item update messages.
     */
    void processItemUpdates();

private:
    /** Used for accessing item and inventory data. */
    World& world;
    /** Used for sending requests and receiving item and inventory data. */
    Network& network;
};

} // namespace Client
} // namespace AM
