#include "GraphicSystem.h"
#include "World.h"
#include "GraphicData.h"
#include "ISimulationExtension.h"
#include "Rotation.h"
#include "GraphicState.h"
#include "Input.h"
#include "ClientGraphicState.h"
#include "Log.h"
#include "AMAssert.h"

namespace AM
{
namespace Client
{

// Values commonly used in calculations.
static constexpr Uint8 RUN_SOUTH_VALUE{
    static_cast<Uint8>(EntityGraphicType::RunSouth)};
static constexpr Uint8 IDLE_SOUTH_VALUE{
    static_cast<Uint8>(EntityGraphicType::IdleSouth)};

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

        // Update to either a run or idle graphic type, depending on whether 
        // this entity is moving.
        const EntityGraphicSet& graphicSet{
            graphicData.getEntityGraphicSet(graphicState.graphicSetID)};
        EntityGraphicType newGraphicType{EntityGraphicType::NotSet};
        if (isMoving(entity)) {
            newGraphicType = getUpdatedRunGraphicType(graphicSet, rotation,
                                                      clientGraphicState);
        }
        else {
            newGraphicType = getUpdatedIdleGraphicType(graphicSet, rotation,
                                                       clientGraphicState);
        }

        // If the graphic type has changed, update the component.
        if (newGraphicType != clientGraphicState.graphicType) {
            clientGraphicState.graphicType = newGraphicType;
            clientGraphicState.setStartTime = true;
        }
    }
}

void GraphicSystem::setExtension(ISimulationExtension* inExtension)
{
    extension = std::move(inExtension);
}

bool GraphicSystem::isMoving(entt::entity entity)
{
    // If any inputs are pressed, return true.
    if (const Input* input{world.registry.try_get<Input>(entity)}) {
        if (input->inputStates.any()) {
            return true;
        }
    }

    return false;
}

EntityGraphicType GraphicSystem::getUpdatedRunGraphicType(
    const EntityGraphicSet& graphicSet, const Rotation& rotation,
    const ClientGraphicState& clientGraphicState)
{
    // If the graphic set contains the desired type, use it.
    EntityGraphicType desiredType{toRunGraphicType(rotation.direction)};
    if (graphicSet.graphics.contains(desiredType)) {
        return desiredType;
    }

    // If the new rotation is an ordinal direction and the current graphic 
    // includes that direction, don't change the graphic.
    // This will cause a "strafe" effect when moving ordinally, if the 
    // ordinal graphics are missing but the cardinal are present.
    EntityGraphicType currentType{clientGraphicState.graphicType};
    Rotation::Direction currentDirection{
        toDirection(clientGraphicState.graphicType)};
    bool isDesiredSW{desiredType == EntityGraphicType::RunSouthWest};
    bool isDesiredNW{desiredType == EntityGraphicType::RunNorthWest};
    bool isDesiredNE{desiredType == EntityGraphicType::RunNorthEast};
    bool isDesiredSE{desiredType == EntityGraphicType::RunSouthEast};
    bool isCurrentSouth{currentDirection == Rotation::Direction::South};
    bool isCurrentWest{currentDirection == Rotation::Direction::West};
    bool isCurrentNorth{currentDirection == Rotation::Direction::North};
    bool isCurrentEast{currentDirection == Rotation::Direction::East};
    if ((isDesiredSW && (isCurrentSouth || isCurrentWest))
        || (isDesiredNW && (isCurrentNorth || isCurrentWest))
        || (isDesiredNE && (isCurrentNorth || isCurrentEast))
        || (isDesiredSE && (isCurrentSouth || isCurrentEast))) {
        // Try to use a run graphic. Otherwise, fall back to the idle graphic.
        EntityGraphicType currentTypeAsRun{toRunGraphicType(currentType)};
        EntityGraphicType currentTypeAsIdle{toIdleGraphicType(currentType)};
        if (graphicSet.graphics.contains(currentTypeAsRun)) {
            return currentTypeAsRun;
        }
        else if (graphicSet.graphics.contains(currentTypeAsIdle)) {
            return currentTypeAsIdle;
        }
    }

    // If the new rotation is an ordinal direction, try to use a cardinal 
    // direction. 
    if ((isDesiredSW || isDesiredSE)
        && graphicSet.graphics.contains(EntityGraphicType::RunSouth)) {
        return EntityGraphicType::RunSouth;
    }
    else if ((isDesiredNW || isDesiredNE)
        && graphicSet.graphics.contains(EntityGraphicType::RunNorth)) {
        return EntityGraphicType::RunNorth;
    }

    // If the graphic set contains the idle version of the desired type, use it.
    EntityGraphicType desiredTypeAsIdle{toIdleGraphicType(desiredType)};
    if (graphicSet.graphics.contains(desiredTypeAsIdle)) {
        return desiredTypeAsIdle;
    }

    return EntityGraphicType::IdleSouth;
}

EntityGraphicType GraphicSystem::getUpdatedIdleGraphicType(
    const EntityGraphicSet& graphicSet, const Rotation& rotation,
    const ClientGraphicState& clientGraphicState)
{
    // If the graphic set contains the desired type, use it.
    EntityGraphicType desiredType{toIdleGraphicType(rotation.direction)};
    if (graphicSet.graphics.contains(desiredType)) {
        return desiredType;
    }

    // If the new rotation is an ordinal direction and the current graphic 
    // includes that direction, don't change the graphic.
    EntityGraphicType currentType{clientGraphicState.graphicType};
    EntityGraphicType currentTypeAsIdle{toIdleGraphicType(currentType)};
    bool isDesiredSW{desiredType == EntityGraphicType::IdleSouthWest};
    bool isDesiredNW{desiredType == EntityGraphicType::IdleNorthWest};
    bool isDesiredNE{desiredType == EntityGraphicType::IdleNorthEast};
    bool isDesiredSE{desiredType == EntityGraphicType::IdleSouthEast};
    if (graphicSet.graphics.contains(currentTypeAsIdle)) {
        Rotation::Direction currentDirection{
            toDirection(clientGraphicState.graphicType)};
        bool isCurrentSouth{currentDirection == Rotation::Direction::South};
        bool isCurrentWest{currentDirection == Rotation::Direction::West};
        bool isCurrentNorth{currentDirection == Rotation::Direction::North};
        bool isCurrentEast{currentDirection == Rotation::Direction::East};
        if ((isDesiredSW && (isCurrentSouth || isCurrentWest))
            || (isDesiredNW && (isCurrentNorth || isCurrentWest))
            || (isDesiredNE && (isCurrentNorth || isCurrentEast))
            || (isDesiredSE && (isCurrentSouth || isCurrentEast))) {
            return currentTypeAsIdle;
        }
    }

    // If the new rotation is an ordinal direction, try to use a cardinal 
    // direction. 
    if ((isDesiredSW || isDesiredSE)
        && graphicSet.graphics.contains(EntityGraphicType::IdleSouth)) {
        return EntityGraphicType::IdleSouth;
    }
    else if ((isDesiredNW || isDesiredNE)
        && graphicSet.graphics.contains(EntityGraphicType::IdleNorth)) {
        return EntityGraphicType::IdleNorth;
    }

    return EntityGraphicType::IdleSouth;
}

Rotation::Direction GraphicSystem::toDirection(EntityGraphicType graphicType)
{
    Uint8 graphicTypeValue{static_cast<Uint8>(graphicType)};
    AM_ASSERT(
        graphicTypeValue <= static_cast<Uint8>(EntityGraphicType::RunSouthEast),
        "Tried to convert a graphic type that this function doesn't support.");

    // Determine how much of an offset we need to match the Direction values.
    Uint8 valueOffset{IDLE_SOUTH_VALUE};
    if (graphicTypeValue >= RUN_SOUTH_VALUE) {
        valueOffset = RUN_SOUTH_VALUE;
    }

    return static_cast<Rotation::Direction>(graphicTypeValue - valueOffset);
}

EntityGraphicType
    GraphicSystem::toRunGraphicType(Rotation::Direction direction)
{
    return static_cast<EntityGraphicType>(direction + RUN_SOUTH_VALUE);
}

EntityGraphicType GraphicSystem::toRunGraphicType(EntityGraphicType graphicType)
{
    Uint8 graphicTypeValue{static_cast<Uint8>(graphicType)};
    AM_ASSERT(
        graphicTypeValue <= static_cast<Uint8>(EntityGraphicType::RunSouthEast),
        "Tried to convert a graphic type that this function doesn't support.");

    // If it's an Idle value, offset it up into the Run value range.
    if (graphicTypeValue < RUN_SOUTH_VALUE) {
        graphicTypeValue += (RUN_SOUTH_VALUE - IDLE_SOUTH_VALUE);
    }

    return static_cast<EntityGraphicType>(graphicTypeValue);
}

EntityGraphicType
    GraphicSystem::toIdleGraphicType(Rotation::Direction direction)
{
    return static_cast<EntityGraphicType>(direction + IDLE_SOUTH_VALUE);
}

EntityGraphicType
    GraphicSystem::toIdleGraphicType(EntityGraphicType graphicType)
{
    Uint8 graphicTypeValue{static_cast<Uint8>(graphicType)};
    AM_ASSERT(
        graphicTypeValue <= static_cast<Uint8>(EntityGraphicType::RunSouthEast),
        "Tried to convert a graphic type that this function doesn't support.");

    // If it's a Run value, offset it down into the Idle value range.
    if (graphicTypeValue >= RUN_SOUTH_VALUE) {
        graphicTypeValue -= (RUN_SOUTH_VALUE - IDLE_SOUTH_VALUE);
    }

    return static_cast<EntityGraphicType>(graphicTypeValue);
}

} // End namespace Client
} // End namespace AM
