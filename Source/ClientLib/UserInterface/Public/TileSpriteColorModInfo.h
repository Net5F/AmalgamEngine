#pragma once

#include "TileLayers.h"
#include <SDL_pixels.h>

namespace AM
{
namespace Client
{

/**
 * Used by the UI when it wants a tile map sprite to have its color and/or 
 * transparency modified.
 */
struct TileSpriteColorModInfo {
    /** The X coordinate of the tile to add the phantom to. */
    int tileX{0};

    /** The Y coordinate of the tile to add the phantom to. */
    int tileY{0};

    /** The phantom's tile layer type. */
    TileLayer::Type layerType{TileLayer::Type::None};

    /** The sprite to modify. */
    const Sprite* sprite{nullptr};

    /** The color and transparency to multiply the sprite by. */
    SDL_Color colorMod{255, 255, 255, 255};
};

} // End namespace Client
} // End namespace AM
