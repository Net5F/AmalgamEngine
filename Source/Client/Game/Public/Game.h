#ifndef GAME_H
#define GAME_H

#include "World.h"
#include "PlayerInputSystem.h"
#include "NetworkMovementSystem.h"
#include "MovementSystem.h"
#include "Message_generated.h"

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
    static constexpr float GAME_TICK_INTERVAL_S = 1 / 60.0f;

    /** 60 render ticks per seconds. */
    static constexpr float RENDER_TICK_INTERVAL_S = 1 / 60.0f;

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

    Uint32 getCurrentTick();

private:
    World world;
    Network& network;

    PlayerInputSystem playerInputSystem;
    NetworkMovementSystem networkMovementSystem;
    MovementSystem movementSystem;

    flatbuffers::FlatBufferBuilder builder;

    /** The aggregated time since we last processed a tick. */
    float timeSinceTick;

    /**
     * The number of the tick that we're currently on.
     * Initialized to the number that the server tells us it's on.
     */
    Uint32 currentTick;

    // Temporary until a resource manager is created.
    std::shared_ptr<SDL2pp::Texture>& sprites;
};

} // namespace Client
} // namespace AM

#endif /* GAME_H */