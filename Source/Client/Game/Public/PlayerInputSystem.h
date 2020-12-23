#pragma once

#include "GameDefs.h"
#include "Network.h"
#include "SDL_events.h"

namespace AM
{
namespace Client
{
class Game;
class World;
class Network;

class PlayerInputSystem
{
public:
    PlayerInputSystem(Game& inGame, World& inWorld);

    /**
     * Updates the player's input state with the given event.
     */
    void processMomentaryInput(SDL_Event& event);

    /**
     * Processes held inputs (movement, etc).
     * @pre SDL_PollEvent or SDL_PumpEvents must have been recently called.
     */
    void processHeldInputs();

    /**
     * Adds the current player input state to the world's playerInputHistory.
     */
    void addCurrentInputsToHistory();

private:
    Game& game;
    World& world;
};

} // namespace Client
} // namespace AM
