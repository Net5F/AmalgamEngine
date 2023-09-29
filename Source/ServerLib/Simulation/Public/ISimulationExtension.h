#pragma once

#include "OSEventHandler.h"
#include "NetworkDefs.h"

namespace AM
{
struct TileUpdateRequest;
struct TileExtent;
struct EntityInitRequest;
struct EntityDeleteRequest;
struct NameChangeRequest;
struct AnimationStateChangeRequest;

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
     * Called after all relevant state has been sent to the clients.
     */
    virtual void afterClientSync() = 0;

    /**
     * Called after all other systems.
     */
    virtual void afterAll() = 0;

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
    // These functions allow the project to place constraints on various 
    // World state modifications.
    /** @return true if the given extent is editable, else false. */
    virtual bool isTileExtentEditable(NetworkID netID,
                                      const TileExtent& tileExtent) const
        = 0;
    /** @return true if the given request is valid, else false. */
    virtual bool isEntityInitRequestValid(
        const EntityInitRequest& entityInitRequest) const
        = 0;
    virtual bool isEntityDeleteRequestValid(
        const EntityDeleteRequest& entityDeleteRequest) const
        = 0;
    virtual bool isNameChangeRequestValid(
        const NameChangeRequest& nameChangeRequest) const
        = 0;
    virtual bool isAnimationStateChangeRequestValid(
        const AnimationStateChangeRequest& nameChangeRequest) const
        = 0;
};

} // namespace Server
} // namespace AM
