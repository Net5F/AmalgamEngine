#ifndef NETWORKOUTPUTSYSTEM_H
#define NETWORKOUTPUTSYSTEM_H

#include "Message_generated.h"
#include "SharedDefs.h"

namespace AM
{
namespace Server
{

class Game;
class World;
class Network;

/**
 *
 */
class NetworkOutputSystem
{
public:
    /** 30 game ticks per second. */
    static constexpr float NETWORK_OUTPUT_TICK_INTERVAL_S = 1 / 20.0f;

    NetworkOutputSystem(Game& inGame, World& inWorld, Network& inNetwork);

    /**
     * Sends dirty entity state data to all clients.
     */
    void updateClients(float deltaSeconds);

private:
    /**
     * Sends the given entity's relevant state information to all connected clients.
     */
    void broadcastEntity(EntityID entityID);

    Game& game;
    World& world;
    Network& network;

    flatbuffers::FlatBufferBuilder builder;

    /** The aggregated time since we last processed a tick. */
    float timeSinceTick;
};

} // namespace Server
} // namespace AM

#endif /* NETWORKOUTPUTSYSTEM_H */
