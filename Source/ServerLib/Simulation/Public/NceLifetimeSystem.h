#pragma once

#include "DynamicObjectCreateRequest.h"
#include "EntityDelete.h"
#include "QueuedEvents.h"

namespace AM
{
namespace Server
{

class World;
class Network;
class SpriteData;

/**
 * Manages creation and destruction of non-client entities.
 *
 * Non-client entities are any entity not controlled by a client (dynamic 
 * objects, NPCs, etc).
 */
class NceLifetimeSystem
{
public:
    NceLifetimeSystem(World& inWorld,
                      EventDispatcher& inNetworkEventDispatcher,
                      Network& inNetwork, SpriteData& inSpriteData);

    /**
     * Processes any waiting EntityCreateRequest or EntityDelete messages.
     */
    void processUpdates();

private:
    /** Used to add/remove entities. */
    World& world;

    /** Used to send entity-related messages. */
    Network& network;

    /** Used to get sprite data when adding an entity. */
    SpriteData& spriteData;

    EventQueue<DynamicObjectCreateRequest> objectCreateRequestQueue;
    EventQueue<EntityDelete> deleteQueue;
};

} // End namespace Server
} // End namespace AM
