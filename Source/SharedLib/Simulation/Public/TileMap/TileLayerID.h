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
 * set ID and sprite index, but they'll be exactly the same so differentiation
 * isn't useful.
 */
struct TileLayerID {
    /** The position of the tile that contains the layer. */
    TilePosition tilePosition{};

    /** The tile layer's type. */
    TileLayer::Type type{TileLayer::Type::None};

    /** The numeric ID of the layer's graphic set. */
    Uint16 graphicSetID{0};

    /** The index of the layer's graphic, within the layer's graphic set.
        For Floors, this will always be 0.
        For FloorCoverings and Objects, this should be cast to Rotation.
        For Walls, this should be cast to Wall::Type. */
    Uint8 graphicIndex{0};

    bool operator==(const TileLayerID& other) const
    {
        return (tilePosition.x == other.tilePosition.x)
               && (tilePosition.y == other.tilePosition.y)
               && (tilePosition.z == other.tilePosition.z)
               && (type == other.type) && (graphicSetID == other.graphicSetID)
               && (graphicIndex == other.graphicIndex);
    }
};

} // End namespace AM
