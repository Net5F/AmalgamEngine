#include "GraphicSystem.h"
#include "World.h"
#include "GraphicData.h"
#include "ISimulationExtension.h"
#include "Rotation.h"
#include "GraphicState.h"
#include "ClientGraphicState.h"
#include "Input.h"
#include "ClientCastState.h"
#include "GraphicHelpers.h"
#include "Log.h"
#include "AMAssert.h"

namespace AM
{
namespace Client
{

GraphicSystem::GraphicSystem(World& inWorld, GraphicData& inGraphicData)
: world{inWorld}
, graphicData{inGraphicData}
, extension{nullptr}
{
}

void GraphicSystem::updateAnimations()
{
    // Update all entity sprites to match their current rotation.
    // Note: This iterates static entities, even though most don't change state
    //       very often. If this becomes a performance issue, we can revisit.
    auto view{world.registry.view<Rotation, GraphicState, ClientGraphicState>()};
    for (auto [entity, rotation, graphicState, clientGraphicState] :
         view.each()) {
        // Give the project a chance to update the graphic type.
        if (extension) {
            EntityGraphicType graphicType{
                extension->getUpdatedGraphicType(entity)};
            if (graphicType != EntityGraphicType::NotSet) {
                clientGraphicState.graphicType = graphicType;
                clientGraphicState.setStartTime = true;
                continue;
            }
        }

        // Determine which graphic type is desired, based on the current 
        // entity state.
        EntityGraphicType desiredGraphicType{getDesiredGraphicType(entity)};

        // Get the new graphic, accounting for missing graphics in the set.
        const EntityGraphicSet& graphicSet{
            graphicData.getEntityGraphicSet(graphicState.graphicSetID)};
        GraphicHelpers::GraphicReturn newGraphic{
            GraphicHelpers::getGraphicOrFallback(
                graphicSet, clientGraphicState.graphicType,
                clientGraphicState.graphicDirection, desiredGraphicType,
                rotation.direction)};

        // If the graphic has changed, update the component.
        if (newGraphic.type != clientGraphicState.graphicType) {
            clientGraphicState.graphicType = newGraphic.type;
            clientGraphicState.setStartTime = true;
        }
        if (newGraphic.direction != clientGraphicState.graphicDirection) {
            clientGraphicState.graphicDirection = newGraphic.direction;
            // Note: We don't reset the animation time, since we want e.g. 
            //       a run animation to play smoothly when changing direction.
        }
    }
}

void GraphicSystem::setExtension(ISimulationExtension* inExtension)
{
    extension = std::move(inExtension);
}

EntityGraphicType GraphicSystem::getDesiredGraphicType(entt::entity entity)
{
    // If the entity is moving, use a run graphic.
    if (const Input* input{world.registry.try_get<Input>(entity)};
        input && input->inputStates.any()) {
        return EntityGraphicType::Run;
    }
    // If the entity is casting, use the appropriate graphic from the 
    // Castable.
    else if (const auto* castState{
                 world.registry.try_get<ClientCastState>(entity)}) {
        if (castState->state == ClientCastState::State::Casting) {
            return castState->castInfo.castable->castingGraphicType;
        }
        else {
            // CastComplete
            return castState->castInfo.castable->castCompleteGraphicType;
        }
    }

    // Default: Use an idle graphic.
    return EntityGraphicType::Idle;
}

} // End namespace Client
} // End namespace AM
