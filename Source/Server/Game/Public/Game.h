#ifndef GAME_H
#define GAME_H

#include "World.h"
#include "NetworkInputSystem.h"
#include "MovementSystem.h"
#include "NetworkOutputSystem.h"

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

    /**
     * Processes all newly connected clients, adding them to the sim.
     */
    void processConnectEvents();

    /**
     * Processes all newly disconnected clients, removing them from the sim.
     */
    void processDisconnectEvents();

    Uint32 getCurrentTick();

private:
    World world;
    Network& network;

    NetworkInputSystem networkInputSystem;
    MovementSystem movementSystem;
    NetworkOutputSystem networkOutputSystem;

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
