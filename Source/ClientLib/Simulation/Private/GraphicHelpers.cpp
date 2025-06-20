#include "GraphicHelpers.h"

namespace AM
{
namespace Client
{

GraphicHelpers::GraphicReturn GraphicHelpers::getGraphicOrFallback(
    const EntityGraphicSet& graphicSet, EntityGraphicType currentType,
    Rotation::Direction currentDirection, EntityGraphicType desiredType,
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
        if (graphicSet.contains(currentType, currentDirection)) {
            return {currentType, currentDirection};
        }
        else if (graphicSet.contains(EntityGraphicType::Idle,
                                     currentDirection)) {
            return {EntityGraphicType::Idle, currentDirection};
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
