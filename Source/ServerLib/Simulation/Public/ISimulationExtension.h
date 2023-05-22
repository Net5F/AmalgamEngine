#pragma once

#include "OSEventHandler.h"

namespace AM
{
struct TileUpdateRequest;
struct TileExtent;

namespace Server
{

/**
 * Defines an extension for the engine's Server::Simulation class.
 *
 * Extensions are implemented by the project, and are given generic functions
 * ("hooks") in which they can implement relevant project logic.
 *
 * The project can register the extension class with the engine through
 * Application::registerSimulationExtension().
 */
class ISimulationExtension : public OSEventHandler
{
public:
    // Canonical constructor (derived class must implement):
    // SimulationExtension(SimulationExDependencies deps)

    //-------------------------------------------------------------------------
    // Simulation Tick Hooks (Call your systems in these)
    //-------------------------------------------------------------------------
    /**
     * Called before any systems are ran.
     */
    virtual void beforeAll() = 0;

    /**
     * Called after the tile map is updated and client entities are
     * added/removed.
     */
    virtual void afterMapAndConnectionUpdates() = 0;

    /**
     * Called after all entity movement has been processed, but before the new
     * movement state has been sent to the clients.
     */
    virtual void afterMovement() = 0;

    /**
     * Called after all entity movement state has been sent to the clients.
     */
    virtual void afterMovementSync() = 0;

    /**
     * See OSEventHandler for details.
     *
     * Note: Simulation will pass events to this class first. If the event is
     *       not handled, then Simulation will attempt to handle it.
     */
    bool handleOSEvent(SDL_Event& event) override = 0;

    //-------------------------------------------------------------------------
    // Simulation System Hooks (Hooks into engine systems)
    //-------------------------------------------------------------------------
    /**
     * Called by TileUpdateSystem when a tile update request is received,
     * before applying the update.
     * Allows the project to place constraints on map modifications, such as
     * requiring certain permissions, or only allowing updates to certain areas.
     *
     * @return true if the given extent is editable, else false.
     */
    virtual bool isExtentEditable(const TileExtent& tileExtent) = 0;
};

} // namespace Server
} // namespace AM
