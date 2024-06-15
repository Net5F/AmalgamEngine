#pragma once

#include "TilePosition.h"
#include "TileLayer.h"
#include "GraphicSetIDs.h"
#include <SDL_stdinc.h>

namespace AM
{

/**
 * The minimum information needed to uniquely identify a tile layer.
 *
 * Technically, multiple layers may be the same type and use the same
 * graphic set ID and graphic value, but they'll be exactly the same so 
 * differentiation isn't useful.
 */
struct TileLayerID {
    /** The position of the tile that contains the layer. */
    TilePosition tilePosition{};

    /** The tile layer's type. */
    TileLayer::Type type{TileLayer::Type::None};

    /** The numeric ID of the layer's graphic set. */
    Uint16 graphicSetID{0};

    /** A value that describes this layer's graphic.
        For all types except Terrain, this is simply an index into 
        graphicSet.graphics. For Terrain, this is a bit-packed value.
        For Terrain, cast this to Terrain::Value. For Walls, cast this to 
        Wall::Type. For Floors and Objects, cast this to Rotation::Direction. */
    Uint8 graphicValue{0};

    bool operator==(const TileLayerID& other) const
    {
        return (tilePosition.x == other.tilePosition.x)
               && (tilePosition.y == other.tilePosition.y)
               && (tilePosition.z == other.tilePosition.z)
               && (type == other.type) && (graphicSetID == other.graphicSetID)
               && (graphicValue == other.graphicValue);
    }
};

} // End namespace AM
