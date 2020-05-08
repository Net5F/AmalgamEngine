#ifndef GAME_H
#define GAME_H

#include "World.h"
#include "NetworkInputSystem.h"
#include "MovementSystem.h"
#include "NetworkOutputSystem.h"
#include "Message_generated.h"

namespace AM
{
namespace Server
{

class Network;

/**
 *
 */
class Game
{
public:
    /** 30 game ticks per second. */
    static constexpr float GAME_TICK_INTERVAL_S = 1 / 30.0f;

    Game(Network& inNetwork);

    /**
     * Runs an iteration of the game loop.
     */
    void tick(float deltaSeconds);

    Uint32 getCurrentTick();

private:
    World world;
    Network& network;

    NetworkInputSystem networkInputSystem;
    MovementSystem movementSystem;
    NetworkOutputSystem networkOutputSystem;

    static constexpr int BUILDER_BUFFER_SIZE = 512;
    flatbuffers::FlatBufferBuilder builder;

    /** The aggregated time since we last processed a tick. */
    float accumulatedTime;

    /**
     * The number of the tick that we're currently on.
     */
    Uint32 currentTick;
};

} // namespace Server
} // namespace AM

#endif /* GAME_H */
