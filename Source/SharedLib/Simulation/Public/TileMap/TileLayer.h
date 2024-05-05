#pragma once

#include "GraphicRef.h"
#include "Wall.h"
#include <span>

namespace AM
{

struct GraphicSet;

/**
 * A single graphic layer of a tile.
 */
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

    /** This layer's type.
        This must always be set to a valid type (not None). */
    Type type{};

    /** The index within graphicSet->graphics of the graphic that is on this 
        layer.
        For Walls, cast this to Wall::Type. For Floor Coverings and Objects,
        cast this to Rotation::Direction. For Floors, this will always be 0
        (floor graphic sets only have 1 graphic).
        Note: It'd be more intuitive to put this after graphicSet, but alignment
              would cause this struct to be larger if we did so. */
    Uint8 graphicIndex{0};

    /** A polymorphic reference to this layer's graphic set.
        Each layer type maps directly to a single graphic set type, e.g. Floor 
        layers -> FloorGraphicSet. */
    std::reference_wrapper<const GraphicSet> graphicSet;

    /**
     * Casts graphicSet to the appropriate type and returns 
     * graphicSet.graphics[graphicIndex].
     */
    GraphicRef getGraphic() const;
};

} // End namespace AM
