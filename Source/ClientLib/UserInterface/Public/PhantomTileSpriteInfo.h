#pragma once

#include "TileLayers.h"
#include "Wall.h"

namespace AM
{
struct Sprite;
namespace Client
{

/**
 * Used by the UI when it wants a "phantom" tile sprite to be shown to the user.
 * 
 * Phantoms are visual-only, so they're purely a UI/Renderer concern and don't 
 * get added to the Simulation's tile map.
 *
 * An example usage is when a user is in build mode and is trying to place a 
 * wall. A phantom wall sprite will follow the user's mouse to show where the 
 * wall will be placed when they click.
 */
struct PhantomTileSpriteInfo {
    /** The X coordinate of the tile to add the phantom to. */
    int tileX{0};

    /** The Y coordinate of the tile to add the phantom to. */
    int tileY{0};

    /** The phantom's tile layer type. If this type is Floor or Wall and the 
        tile already has a sprite in that position, this phantom will visually 
        replace it. */
    TileLayer::Type layerType{TileLayer::Type::None};

    /** If layerType == Wall, this is the type of wall. */
    Wall::Type wallType{Wall::Type::None};

    /** The phantom sprite to add. */
    const Sprite* sprite{nullptr};
};

} // End namespace Client
} // End namespace AM
