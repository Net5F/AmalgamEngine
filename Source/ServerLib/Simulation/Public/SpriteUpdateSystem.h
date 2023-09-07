#pragma once

#include "SpriteChange.h"
#include "entt/fwd.hpp"
#include "QueuedEvents.h"

namespace AM
{
namespace Server
{
class Simulation;
class World;
class Network;
class SpriteData;

// TODO: When we add animation support, maybe call this something like 
//       AnimationSetUpdateSystem.
/**
 * Processes sprite change requests sent by clients. If a request is valid, 
 * updates the entity's sprite state.
 * Also, detects changes to sprite state and sends the new state to all 
 * nearby clients.
 */
class SpriteUpdateSystem
{
public:
    SpriteUpdateSystem(Simulation& inSimulation, World& inWorld,
                       Network& inNetwork, SpriteData& inSpriteData);

    /**
     * Processes sprite change requests.
     */
    void updateSprites();

    /**
     * Sends any dirty sprite state to all nearby clients.
     */
    void sendSpriteUpdates();

private:
    /** Used to get the current tick number. */
    Simulation& simulation;
    /** Used for fetching sprite data. */
    World& world;
    /** Used for receiving change requests and sending sprites to clients. */
    Network& network;
    /** Used for getting our sprites and sprite sets. */
    SpriteData& spriteData;

    EventQueue<SpriteChange> spriteChangeQueue;
};

} // namespace Server
} // namespace AM
