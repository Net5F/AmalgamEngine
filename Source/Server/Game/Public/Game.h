#ifndef GAME_H
#define GAME_H

#include "World.h"
#include "NetworkInputSystem.h"
#include "MovementSystem.h"
#include "Message_generated.h"

namespace AM
{

class NetworkServer;

/**
 *
 */
class Game
{
public:
    /**
     * The amount of time in milliseconds that we wait between ticks.
     * We currently target 30 server ticks per second.
     */
    static constexpr double TICK_INTERVAL_MS = 33.3;

    Game(NetworkServer& inNetwork);

    /**
     * Runs an iteration of the game loop.
     */
    void tick(double deltaMs);

private:
    World world;
    NetworkServer& network;

    NetworkInputSystem networkInputSystem;
    MovementSystem movementSystem;

    flatbuffers::FlatBufferBuilder builder;

    /** The aggregated time since we last processed a tick. */
    double timeSinceTick;
};

} // namespace AM

#endif /* GAME_H */
