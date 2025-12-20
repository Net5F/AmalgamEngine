#pragma once

#include "Network.h"
#include <SDL_events.h>

namespace AM
{
namespace Client
{
struct SimulationContext;
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
    PlayerInputSystem(const SimulationContext& inSimContext);

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
    /**
     * Processes any mouse wheel movement since the last tick.
     */
    void processMouseWheel(SDL_MouseWheelEvent& wheelEvent);

    Simulation& simulation;
    World& world;
    Network& network;

    /** The index within ZOOM_LEVELS that is currently selected. */
    std::size_t currentZoomLevelIndex;
};

} // namespace Client
} // namespace AM
