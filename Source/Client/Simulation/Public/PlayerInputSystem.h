#pragma once

#include "Network.h"
#include <SDL_events.h>

namespace AM
{
namespace Client
{
class Simulation;
class World;
class Network;

/**
 * Processes the player's input state.
 *
 * Performs updates locally, and sends state change requests to the server.
 * If a change is refused, PlayerMovementSystem will handle the resolution of
 * our local state.
 */
class PlayerInputSystem
{
public:
    PlayerInputSystem(Simulation& inSim, World& inWorld, Network& inNetwork);

    /**
     * Updates the player's input state with the given event.
     */
    void processMomentaryInput(SDL_Event& event);

    /**
     * Processes the mouse's current position and click states.
     */
    void processMouseState(SDL_MouseMotionEvent& event);

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
    /**
     * Processes any mouse wheel movement since the last tick.
     */
    void processMouseWheel(SDL_MouseWheelEvent& wheelEvent);

    Simulation& sim;
    World& world;
    Network& network;
};

} // namespace Client
} // namespace AM
