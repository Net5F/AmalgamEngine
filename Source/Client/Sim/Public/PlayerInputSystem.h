#pragma once

#include "SimDefs.h"
#include "Network.h"
#include "SDL_events.h"

namespace AM
{
namespace Client
{
class Sim;
class World;
class Network;

class PlayerInputSystem
{
public:
    PlayerInputSystem(Sim& inSim, World& inWorld);

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
    Sim& sim;
    World& world;
};

} // namespace Client
} // namespace AM
