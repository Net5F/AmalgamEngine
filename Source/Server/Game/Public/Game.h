#ifndef GAME_H
#define GAME_H

#include "World.h"
#include "NetworkInputSystem.h"
#include "MovementSystem.h"
#include "NetworkOutputSystem.h"
#include <atomic>

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
    /** An unreasonable amount of time for the game tick to be late by. */
    static constexpr float GAME_DELAYED_TIME_S = .001;

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
    std::atomic<Uint32> currentTick;
};

} // namespace Server
} // namespace AM

#endif /* GAME_H */
