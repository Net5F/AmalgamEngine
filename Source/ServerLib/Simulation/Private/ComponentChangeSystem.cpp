#include "ComponentChangeSystem.h"
#include "World.h"
#include "Network.h"
#include "GraphicData.h"
#include "Collision.h"
#include "ClientSimData.h"
#include "ISimulationExtension.h"
#include "Transforms.h"
#include "Log.h"
#include "tracy/Tracy.hpp"

namespace AM
{
namespace Server
{
ComponentChangeSystem::ComponentChangeSystem(World& inWorld, Network& inNetwork,
                                             GraphicData& inGraphicData)
: world{inWorld}
, network{inNetwork}
, graphicData{inGraphicData}
, extension{nullptr}
, entityNameChangeRequestQueue{inNetwork.getEventDispatcher()}
, graphicStateChangeRequestQueue{inNetwork.getEventDispatcher()}
{
    world.registry.on_update<GraphicState>()
        .connect<&ComponentChangeSystem::onGraphicStateUpdated>(this);
}

void ComponentChangeSystem::processChangeRequests()
{
    ZoneScoped;

    entt::registry& registry{world.registry};

    // Process any waiting update requests.
    EntityNameChangeRequest nameChangeRequest{};
    while (entityNameChangeRequestQueue.pop(nameChangeRequest)) {
        // If the entity isn't valid, skip it.
        if (!(registry.valid(nameChangeRequest.entity))) {
            continue;
        }
        // If the project says the request isn't valid, skip it.
        else if ((extension != nullptr)
                 && !(extension->isEntityNameChangeRequestValid(
                     nameChangeRequest))) {
            continue;
        }

        registry.replace<Name>(nameChangeRequest.entity,
                               nameChangeRequest.name);
    }

    GraphicStateChangeRequest graphicStateChangeRequest{};
    while (graphicStateChangeRequestQueue.pop(graphicStateChangeRequest)) {
        // If the entity isn't valid, skip it.
        if (!(registry.valid(graphicStateChangeRequest.entity))) {
            continue;
        }
        // If the project says the request isn't valid, skip it.
        else if ((extension != nullptr)
                 && !(extension->isGraphicStateChangeRequestValid(
                     graphicStateChangeRequest))) {
            continue;
        }

        registry.replace<GraphicState>(
            graphicStateChangeRequest.entity,
            graphicStateChangeRequest.graphicState);
    }
}

void ComponentChangeSystem::setExtension(ISimulationExtension* inExtension)
{
    extension = inExtension;
}

void ComponentChangeSystem::onGraphicStateUpdated(entt::registry& registry,
                                                  entt::entity entity)
{
    // Since the graphic state was updated, we need to update the entity's
    // collision.
    auto [position, graphicState]
        = registry.get<Position, GraphicState>(entity);
    GraphicRef newGraphic{
        graphicData.getObjectGraphicSet(graphicState.graphicSetID)
            .graphics[graphicState.graphicIndex]};

    // Note: We assume that an entity with GraphicState always has a
    //       Collision.
    const BoundingBox& modelBounds{newGraphic.getModelBounds()};
    const Collision& collision{
        registry.patch<Collision>(entity, [&](Collision& collision) {
            collision.modelBounds = modelBounds;
            collision.worldBounds = Transforms::modelToWorldCentered(
                modelBounds, position);
        })};

    world.entityLocator.setEntityLocation(entity, collision.worldBounds);
}

} // namespace Server
} // namespace AM