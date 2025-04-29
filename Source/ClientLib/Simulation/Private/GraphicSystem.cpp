#include "GraphicSystem.h"
#include "World.h"
#include "GraphicData.h"
#include "ISimulationExtension.h"
#include "Rotation.h"
#include "GraphicState.h"
#include "Input.h"
#include "ClientGraphicState.h"
#include "ClientCastState.h"
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
        GraphicReturn newGraphic{
            getGraphicOrDefault(graphicSet, clientGraphicState,
                                desiredGraphicType, rotation.direction)};

        // If the graphic has changed, update the component.
        if ((newGraphic.type != clientGraphicState.graphicType)
            || (newGraphic.direction != clientGraphicState.graphicDirection)) {
            clientGraphicState.graphicType = newGraphic.type;
            clientGraphicState.graphicDirection = newGraphic.direction;
            clientGraphicState.setStartTime = true;
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

GraphicSystem::GraphicReturn GraphicSystem::getGraphicOrDefault(
    const EntityGraphicSet& graphicSet,
    const ClientGraphicState& clientGraphicState, EntityGraphicType desiredType,
    Rotation::Direction desiredDirection)
{
    // If the graphic set contains the desired graphic, use it.
    if (graphicSet.contains(desiredType, desiredDirection)) {
        return {desiredType, desiredDirection};
    }

    // The set doesn't have the desired graphic. Find the closest match.

    // #### Ordinal Strafing ####
    // If the new rotation is an ordinal direction and the current direction 
    // includes that direction, don't change it.
    // This will cause a "strafe" effect when moving ordinally, if the 
    // ordinal graphics are missing but the cardinal are present.
    EntityGraphicType currentType{clientGraphicState.graphicType};
    Rotation::Direction currentDirection{clientGraphicState.graphicDirection};
    bool isDesiredSW{(desiredDirection == Rotation::Direction::SouthWest)};
    bool isDesiredNW{(desiredDirection == Rotation::Direction::NorthWest)};
    bool isDesiredNE{(desiredDirection == Rotation::Direction::NorthEast)};
    bool isDesiredSE{(desiredDirection == Rotation::Direction::SouthEast)};
    bool isCurrentSouth{currentDirection == Rotation::Direction::South};
    bool isCurrentWest{currentDirection == Rotation::Direction::West};
    bool isCurrentNorth{currentDirection == Rotation::Direction::North};
    bool isCurrentEast{currentDirection == Rotation::Direction::East};
    if ((isDesiredSW && (isCurrentSouth || isCurrentWest))
        || (isDesiredNW && (isCurrentNorth || isCurrentWest))
        || (isDesiredNE && (isCurrentNorth || isCurrentEast))
        || (isDesiredSE && (isCurrentSouth || isCurrentEast))) {
        // Try to use the current graphic. Otherwise, fall back to the Idle 
        // graphic (the graphic set may have changed).
        if (graphicSet.contains(clientGraphicState.graphicType,
                                clientGraphicState.graphicDirection)) {
            return {clientGraphicState.graphicType,
                    clientGraphicState.graphicDirection};
        }
        else if (graphicSet.contains(EntityGraphicType::Idle,
                                     clientGraphicState.graphicDirection)) {
            return {EntityGraphicType::Idle,
                    clientGraphicState.graphicDirection};
        }
    }

    // #### Ordinal -> Cardinal ####
    // If the new rotation is an ordinal direction, try to use a cardinal 
    // direction. 
    if ((isDesiredSW || isDesiredSE)
        && graphicSet.contains(desiredType, Rotation::Direction::South)) {
        return {desiredType, Rotation::Direction::South};
    }
    else if ((isDesiredNW || isDesiredNE)
             && graphicSet.contains(desiredType, Rotation::Direction::North)) {
        return {desiredType, Rotation::Direction::North};
    }

    // #### Idle With Desired Direction ####
    // If the graphic set contains an Idle graphic for the desired direction, 
    // use it.
    if (graphicSet.contains(EntityGraphicType::Idle, desiredDirection)) {
        return {EntityGraphicType::Idle, desiredDirection};
    }

    // #### Default: Idle South ####
    return {EntityGraphicType::Idle, Rotation::Direction::South};
}

} // End namespace Client
} // End namespace AM
