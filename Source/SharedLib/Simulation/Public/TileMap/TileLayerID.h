#pragma once

#include "TileLayers.h"
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
    /** The tile's X coordinate. */
    int x{0};

    /** The tile's Y coordinate. */
    int y{0};

    /** The tile layer's type. */
    TileLayer::Type type{TileLayer::Type::None};

    /** The numeric ID of the layer's sprite set. */
    Uint16 spriteSetID{0};

    /** The index of the layer's sprite, within the layer's sprite set.
        For Floors, this will always be 0.
        For FloorCoverings and Objects, this should be cast to Rotation.
        For Walls, this should be cast to Wall::Type. */
    Uint8 spriteIndex{0};

    bool operator==(const TileLayerID& other) const
    {
        return (x == other.x) && (y == other.y) && (type == other.type)
               && (spriteSetID == other.spriteSetID)
               && (spriteIndex == other.spriteIndex);
    }
};

} // End namespace AM
