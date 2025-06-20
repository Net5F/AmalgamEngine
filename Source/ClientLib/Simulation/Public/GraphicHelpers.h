#pragma once

#include "GraphicSets.h"
#include "EntityGraphicType.h"
#include "Rotation.h"

namespace AM
{
namespace Client
{
/**
 * Shared static functions for updating graphics.
 */
class GraphicHelpers
{
public:
    struct GraphicReturn {
        EntityGraphicType type{};
        Rotation::Direction direction{};
    };
    /**
     * Tries to return a graphic from graphicSet, matching the desired graphic 
     * type and direction. If the set doesn't have a graphic in that slot, 
     * returns the closest matching graphic.
     */
    static GraphicReturn getGraphicOrFallback(
        const EntityGraphicSet& graphicSet, EntityGraphicType currentType,
        Rotation::Direction currentDirection, EntityGraphicType desiredType,
        Rotation::Direction desiredDirection);
};

} // End namespace Client
} // End namespace AM

