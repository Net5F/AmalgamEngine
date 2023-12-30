#pragma once

#include "TileLayers.h"
#include "Wall.h"
#include "Position.h"
#include "Sprite.h"

namespace AM
{
struct Sprite;
namespace Client
{

/**
 * Used by the UI when it wants a "phantom" sprite to be shown to the user.
 *
 * This can either be a tile phantom, or an entity phantom (see layerType's
 * comment).
 *
 * Phantoms are visual-only, so they're purely a UI/Renderer concern and don't
 * get added to the simulation.
 *
 * Floor and Wall tile layer phantoms will replace any sprites in the same spot.
 * Entities and other tile layer types get added on top instead of replacing.
 *
 * An example usage is when a user is in build mode and is trying to place a
 * wall. A phantom wall sprite will follow the user's mouse to show where the
 * wall will be placed when they click.
 */
struct PhantomSpriteInfo {
    /** If this is a tile phantom, this is the X coordinate of the phantom's
        tile. */
    int tileX{0};

    /** If this is a tile phantom, this is the Y coordinate of the phantom's
        tile. */
    int tileY{0};

    /** The phantom's tile layer type. If this != None, this is a tile phantom.
        If this == None, this is an entity phantom. */
    TileLayer::Type layerType{TileLayer::Type::None};

    /** If layerType == Wall, this is the type of wall. */
    Wall::Type wallType{Wall::Type::None};

    /** If this is an entity phantom (layerType == None), this is the phantom's
        world position. */
    Position position{};

    /** The phantom sprite to add. */
    const Sprite* sprite{nullptr};
};

} // End namespace Client
} // End namespace AM
