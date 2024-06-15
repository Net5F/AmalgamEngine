#pragma once

#include "GraphicRef.h"
#include "Wall.h"
#include "Terrain.h"
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
        Terrain,
        /** Floors are display-only, they have no collision. They're used for 
            things like grass, carpets, flooring, etc. */
        Floor,
        Wall,
        Object,
        Count,
        None
    };

    /** This layer's type.
        This must always be set to a valid type (not None). */
    Type type{};

    /** A value that describes this layer's graphic.
        For all types except Terrain, this is simply an index into 
        graphicSet.graphics. For Terrain, this is a bit-packed value.
        For Terrain, cast this to Terrain::Value. For Walls, cast this to 
        Wall::Type. For Floors and Objects, cast this to Rotation::Direction.
        Note: It'd be more intuitive to put this after graphicSet, but alignment
              would cause this struct to be larger if we did so. */
    Uint8 graphicValue{0};

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
