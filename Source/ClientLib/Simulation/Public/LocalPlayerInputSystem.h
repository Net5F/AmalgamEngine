#pragma once

#include "Network.h"
#include <SDL3/SDL_events.h>

namespace AM
{
namespace Client
{
struct SimulationContext;
class Simulation;
class World;
class Network;

/**
 * Processes the local player's input state.
 *
 * Performs updates locally, and sends state change requests to the server.
 * If a change is refused, LocalPlayerMovementSystem will handle the resolution
 * of our local state.
 */
class LocalPlayerInputSystem
{
public:
    LocalPlayerInputSystem(const SimulationContext& inSimContext);

    /**
     * Updates the local player's input state with the given event.
     */
    void processMomentaryInput(SDL_Event& event);

    /**
     * Processes held inputs (movement, etc).
     * @pre SDL_PollEvent or SDL_PumpEvents must have been recently called.
     */
    void processHeldInputs();

    /**
     * Adds the local player's current input state to its InputHistory.
     */
    void addCurrentInputsToHistory();

private:
    /**
     * If valid, zooms the camera in by one zoom level.
     */
    void handleZoomIn();

    /**
     * If valid, zooms the camera out by one zoom level.
     */
    void handleZoomOut();

    Simulation& simulation;
    World& world;
    Network& network;

    /** The index within ZOOM_LEVELS that is currently selected. */
    std::size_t currentZoomLevelIndex;
};

} // namespace Client
} // namespace AM
