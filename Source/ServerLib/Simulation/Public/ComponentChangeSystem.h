#pragma once

#include "EntityNameChangeRequest.h"
#include "GraphicStateChangeRequest.h"
#include "QueuedEvents.h"

namespace AM
{
namespace Server
{
class World;
class Network;
class GraphicData;
class ISimulationExtension;

/**
 * Handles change requests for any random engine components that aren't already
 * handled by another system.
 *
 * We do this so that the project can just send a change message, instead of
 * needing to roll its own system.
 */
class ComponentChangeSystem
{
public:
    ComponentChangeSystem(World& inWorld, Network& inNetwork,
                          GraphicData& inGraphicData);

    /**
     * Processes any waiting component change request messages.
     */
    void processChangeRequests();

    void setExtension(ISimulationExtension* inExtension);

private:
    /**
     * Updates Sprite and Collision components when a GraphicState is
     * updated.
     */
    void onGraphicStateUpdated(entt::registry& registry, entt::entity entity);

    /** Used for fetching component data. */
    World& world;
    /** Used for receiving updates from clients. */
    Network& network;
    /** Used to update Collision components when GraphicState is updated. */
    GraphicData& graphicData;
    /** If non-nullptr, contains the project's simulation extension functions.
        Used for checking if component change requests are valid. */
    ISimulationExtension* extension;

    EventQueue<EntityNameChangeRequest> entityNameChangeRequestQueue;
    EventQueue<GraphicStateChangeRequest> graphicStateChangeRequestQueue;
};

} // namespace Server
} // namespace AM
