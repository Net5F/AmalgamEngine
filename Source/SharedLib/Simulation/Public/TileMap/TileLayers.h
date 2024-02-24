#pragma once

#include "GraphicSets.h"
#include "Wall.h"
#include "Rotation.h"
#include <concepts>

namespace AM
{

struct TileLayer {
    /** The types of layers that our tiles support. */
    enum Type : Uint8 {
        Floor,
        /** Floor coverings are things like rugs, flowers, puddles, etc. */
        FloorCovering,
        Wall,
        Object,
        Count,
        None
    };
};

struct FloorTileLayer {
    /** This floor's graphic set. Since each tile always has a stack-allocated
        floor layer, this will be set to nullptr if this layer has no floor. */
    const FloorGraphicSet* graphicSet{nullptr};

    // Floors currently only support a single sprite, so no enum is needed.
    // Eventually, we may add support for "variations" of a floor, so we could
    // support a randomized floor placement tool in build mode.

    /** Note: May be null if this layer has no floor. */
    std::optional<GraphicRef> getGraphic() const
    {
        if (graphicSet) {
            return graphicSet->graphic;
        }
        else {
            return {};
        }
    }
};

struct FloorCoveringTileLayer {
    /** This floor covering's graphic set. */
    const FloorCoveringGraphicSet* graphicSet{nullptr};

    /** The direction that this floor covering is facing. */
    Rotation::Direction direction{Rotation::Direction::None};

    /** Note: May be null if there's no graphic for the current direction,
              but generally you should avoid setting directions that have
              no graphic. */
    std::optional<GraphicRef> getGraphic() const
    {
        return graphicSet->graphics[direction];
    }
};

struct WallTileLayer {
    /** This wall's graphic set. Since each tile always has 2 stack-allocated
        wall layers, this will be set to nullptr if this layer has no wall. */
    const WallGraphicSet* graphicSet{nullptr};

    /** The type of wall that this is. This will be set to None if the tile
        has no wall. */
    Wall::Type wallType{Wall::Type::None};

    /** Note: May be null if this layer has no wall. */
    std::optional<GraphicRef> getGraphic() const
    {
        if (graphicSet) {
            return graphicSet->graphics[wallType];
        }
        else {
            return {};
        }
    };
};

struct ObjectTileLayer {
    /** This object's graphic set. */
    const ObjectGraphicSet* graphicSet{nullptr};

    /** The direction that this object is facing. */
    Rotation::Direction direction{Rotation::Direction::None};

    /** Note: May be null if there's no graphic for the current direction,
              but generally you should avoid setting directions that have
              no graphic. */
    std::optional<GraphicRef> getGraphic() const
    {
        return graphicSet->graphics[direction];
    };
};

/** Concept to match the tile layer types. */
template<typename... T>
concept IsTileLayerType
    = ((std::same_as<
            T,
            FloorTileLayer> || std::same_as<T, FloorCoveringTileLayer> || std::same_as<T, WallTileLayer> || std::same_as<T, ObjectTileLayer>)
       || ...);

} // End namespace AM
