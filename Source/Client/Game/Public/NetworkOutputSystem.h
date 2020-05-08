#ifndef NETWORKOUTPUTSYSTEM_H
#define NETWORKOUTPUTSYSTEM_H

#include "Message_generated.h"
#include "SharedDefs.h"

namespace AM
{
namespace Client
{

class Game;
class World;
class Network;

/**
 * This class is in charge of checking for data that needs to be sent, wrapping it
 * appropriately, and sending it through the Network.
 */
class NetworkOutputSystem
{
public:
    /** 30 game ticks per second. */
    static constexpr float NETWORK_OUTPUT_TICK_INTERVAL_S = 1 / 20.0f;

    NetworkOutputSystem(Game& inGame, World& inWorld, Network& inNetwork);

    /**
     * Sends input state data to the server.
     */
    void updateServer(float deltaSeconds);

private:
    /**
     * If the player inputs have changed, sends them to the server.
     * Else, sends an empty message as a heartbeat.
     * Either way, wraps the message in our wall time to facilitate latency tracking.
     */
    void sendInputState();

    /**
     * Serializes the given entity's relevant world data.
     * @param entityID  The entity to serialize.
     * @return An offset where the data was stored in the builder.
     */
    flatbuffers::Offset<AM::fb::Entity> serializeEntity(EntityID entityID);

    Game& game;
    World& world;
    Network& network;

    static constexpr int BUILDER_BUFFER_SIZE = 512;
    flatbuffers::FlatBufferBuilder builder;

    /** The aggregated time since we last processed a tick. */
    float accumulatedTime;
};

} // namespace Client
} // namespace AM

#endif /* NETWORKOUTPUTSYSTEM_H */
