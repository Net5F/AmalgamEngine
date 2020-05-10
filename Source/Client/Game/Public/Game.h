#ifndef GAME_H
#define GAME_H

#include "NpcMovementSystem.h"
#include "PlayerMovementSystem.h"
#include "World.h"
#include "PlayerInputSystem.h"
#include "NetworkOutputSystem.h"
#include "Message_generated.h"
#include <atomic>

namespace AM
{
namespace Client
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

    Game(Network& inNetwork, std::shared_ptr<SDL2pp::Texture>& inSprites);

    /**
     * Requests to connect to the game server, waits for an assigned EntityID,
     * and constructs the player.
     */
    void connect();

    /**
     * Runs an iteration of the game loop.
     */
    void tick(float deltaSeconds);

    World& getWorld();

    float getAccumulatedTime();

    Uint32 getCurrentTick();

    std::atomic<bool> const* getExitRequestedPtr();

private:
    World world;
    Network& network;

    PlayerInputSystem playerInputSystem;
    NetworkOutputSystem networkOutputSystem;
    PlayerMovementSystem playerMovementSystem;
//    NpcMovementSystem networkMovementSystem;

    /** The aggregated time since we last processed a tick. */
    float accumulatedTime;

    /**
     * The number of the tick that we're currently on.
     * Initialized to the number that the server tells us it's on.
     */
    Uint32 currentTick;

    // Temporary until a resource manager is created.
    std::shared_ptr<SDL2pp::Texture>& sprites;

    /**
     * Turn false to signal that the main loop should end.
     * The game processes the inputs, so it gets to be in charge of program lifespan.
     */
    std::atomic<bool> exitRequested;
};

} // namespace Client
} // namespace AM

#endif /* GAME_H */
