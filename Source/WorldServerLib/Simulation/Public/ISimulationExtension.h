#pragma once

#include "ConnectionHandle.h"
#include "entt/fwd.hpp"

namespace AM
{
struct TileUpdateRequest;
struct TileExtent;
struct EntityInitRequest;
struct EntityDeleteRequest;
struct EntityNameChangeRequest;
struct GraphicStateChangeRequest;
struct ItemInitRequest;
struct ItemChangeRequest;

namespace WorldServer
{
struct ClientEntityLoadInfo;

/**
 * Defines an extension for the engine's WorldServer::Simulation class.
 *
 * Extensions are implemented by the project, and are given generic functions
 * ("hooks") in which they can implement relevant project logic.
 *
 * The project can register the extension class with the engine through
 * Application::registerSimulationExtension().
 */
class ISimulationExtension
{
public:
    // Canonical constructor (derived class must implement):
    // SimulationExtension(const SimulationExDependencies& deps)

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
     * Called after the general simulation update logic (movement, interactions,
     * etc.)
     */
    virtual void afterSimUpdate() = 0;

    /**
     * Called after all relevant state has been sent to the clients.
     */
    virtual void afterClientSync() = 0;

    /**
     * Called after all other systems.
     */
    virtual void afterAll() = 0;

    //-------------------------------------------------------------------------
    // Simulation System Hooks (Hooks into engine systems)
    //-------------------------------------------------------------------------
    // These functions allow the project to affect various World state
    // modifications.

    /** @return true if the given extent is editable, else false. */
    virtual bool isTileExtentEditable(ConnectionHandle clientHandle,
                                      const TileExtent& tileExtent) const = 0;
    /** @return true if the given request is valid, else false. */
    virtual bool isEntityInitRequestValid(
        const EntityInitRequest& entityInitRequest) const = 0;
    virtual bool isEntityDeleteRequestValid(
        const EntityDeleteRequest& entityDeleteRequest) const = 0;
    virtual bool isEntityNameChangeRequestValid(
        const EntityNameChangeRequest& nameChangeRequest) const = 0;
    virtual bool isGraphicStateChangeRequestValid(
        const GraphicStateChangeRequest& graphicStateChangeRequest) const = 0;
    virtual bool
        isItemInitRequestValid(const ItemInitRequest& itemInitRequest) const
        = 0;
    virtual bool isItemChangeRequestValid(
        const ItemChangeRequest& itemChangeRequest) const = 0;

    /**
     * Called when a client entity is added to the world, after its saved
     * components are loaded and the engine's defaults are added.
     *
     * Use this to add defaults for any of your components that weren't
     * loaded (check the registry), or to start a character creation flow.
     *
     * Note: This is called before the entity's IsClientEntity and
     *       ClientSimData components are added.
     * Note: This isn't called when a client takes over its account's
     *       lingering entity, since that entity was already initialized.
     */
    virtual void initClientEntity(entt::entity entity,
                                  const ClientEntityLoadInfo& loadInfo) = 0;
};

} // namespace WorldServer
} // namespace AM
