#include "GraphicSystem.h"
#include "SimulationContext.h"
#include "Simulation.h"
#include "GraphicData.h"
#include "ISimulationExtension.h"
#include "Position.h"
#include "PreviousPosition.h"
#include "Rotation.h"
#include "GraphicState.h"
#include "ClientGraphicState.h"
#include "Movement.h"
#include "Input.h"
#include "ClientCastState.h"
#include "GraphicHelpers.h"
#include "Log.h"
#include "AMAssert.h"

namespace AM
{
namespace Client
{

GraphicSystem::GraphicSystem(const SimulationContext& inSimContext)
: world{inSimContext.simulation.getWorld()}
, graphicData{inSimContext.graphicData}
, extension{nullptr}
{
}

void GraphicSystem::updateAnimations()
{
    // Update all entity sprites to match their current rotation.
    // Note: This iterates static entities, even though most don't change state
    //       very often. If this becomes a performance issue, we can revisit.
    auto view{world.registry.view<Position, PreviousPosition, Rotation,
                                  GraphicState, ClientGraphicState>()};
    for (auto [entity, position, previousPosition, rotation, graphicState,
               clientGraphicState] : view.each()) {
        // Give the project a chance to update the graphic type.
        EntityGraphicType graphicType{extension->getUpdatedGraphicType(entity)};
        if (graphicType != EntityGraphicType::NotSet) {
            clientGraphicState.graphicType = graphicType;
            clientGraphicState.setStartTime = true;
            continue;
        }

        // Determine which graphic type is desired, based on the current
        // entity state.
        EntityGraphicType desiredGraphicType{getDesiredGraphicType(entity)};

        // Get the new graphic, accounting for missing graphics in the set.
        const EntityGraphicSet& graphicSet{
            graphicData.getEntityGraphicSet(graphicState.graphicSetID)};
        bool isMoving{!(position.isEqualApproxWorld(previousPosition))};
        GraphicHelpers::GraphicReturn newGraphic{
            GraphicHelpers::getGraphicOrFallback(
                graphicSet, clientGraphicState.graphicType,
                clientGraphicState.graphicDirection, desiredGraphicType,
                rotation.direction, isMoving)};

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
    entt::registry& registry{world.registry};

    // If the entity is airborne, use an appropriate graphic.
    if (const Movement* movement{registry.try_get<Movement>(entity)};
        movement && movement->isAirborne) {
        // If the entity is falling, use the Fall graphic.
        if (movement->velocity.z < 0.f) {
            return EntityGraphicType::Fall;
        }
        else {
            // Not falling.
            return EntityGraphicType::Jump;
        }
    }

    // If the entity is casting, use the appropriate graphic from the
    // Castable.
    if (const auto* castState{registry.try_get<ClientCastState>(entity)}) {
        if (castState->state == ClientCastState::State::Casting) {
            return castState->castInfo.castable->castingGraphicType;
        }
        else {
            // CastComplete
            return castState->castInfo.castable->castCompleteGraphicType;
        }
    }

    // If the entity has a held input, use an appropriate graphic.
    if (const Input* input{registry.try_get<Input>(entity)};
        input && input->inputStates.any()) {
        if (input->inputStates[Input::Crouch]) {
            return EntityGraphicType::Crouch;
        }

        return EntityGraphicType::Run;
    }

    // Default: Use an idle graphic.
    return EntityGraphicType::Idle;
}

} // End namespace Client
} // End namespace AM
