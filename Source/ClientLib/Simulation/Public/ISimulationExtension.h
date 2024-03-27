#pragma once

#include "OSEventHandler.h"
#include "EntityGraphicType.h"
#include "entt/fwd.hpp"

namespace AM
{
namespace Client
{

/**
 * Defines an extension for the engine's Client::Simulation class.
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
    // SimulationExtension(const SimulationExDependencies& deps)

    /**
     * Initializes or re-initializes our simulation systems.
     *
     * Used to put the systems in a consistent state, so they don't need to
     * account for disconnects/reconnects.
     */
    virtual void initializeSystems() = 0;

    /**
     * Called first in the simulation loop.
     */
    virtual void beforeAll() = 0;

    /**
     * Called after the tile map is updated and peer entities are added/removed.
     */
    virtual void afterMapAndConnectionUpdates() = 0;

    /**
     * Called after the general simulation update logic (movement, interactions,
     * etc.)
     */
    virtual void afterSimUpdate() = 0;

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
    // These functions allow the project to affect various World state 
    // modifications.

    /**
     * Gives the project a chance to update the type of graphic that the given 
     * entity should use, based on its current sim state.
     *
     * The project must handle graphic type updates when the entity is doing 
     * something that the engine isn't aware of. For example, if the project 
     * implements swimming, this function should check if the entity is swimming
     * and return a Swimming graphic type if so.
     * 
     * If this function returns NotSet, the engine will set the entity's graphic 
     * type to one of the types in EngineEntityGraphicType.
     *
     * @return The entity's new graphic type, or NotSet if no project-implemented
     *         graphic types are appropriate.
     */
    virtual EntityGraphicType getUpdatedGraphicType(entt::entity entity) = 0;
};

} // namespace Client
} // namespace AM
