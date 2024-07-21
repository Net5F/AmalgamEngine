#pragma once

#include "TilePosition.h"
#include "TileLayer.h"
#include "TileOffset.h"
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
 * Terrain and Wall tile layer phantoms will replace any sprites in the same 
 * spot. Objects and other tile layer types will be added on top instead of 
 * replacing.
 *
 * An example usage is when a user is in build mode and is trying to place a
 * wall. A phantom wall sprite will follow the user's mouse to show where the
 * wall will be placed when they click.
 *
 * Note: Currently we only support phantom sprites. If we ever want to support 
 *       phantom animations, this should be renamed to PhantomGraphicInfo and 
 *       replace Sprite with GraphicRef.
 */
struct PhantomSpriteInfo {
    /** If this is a tile phantom, this is the position of the phantom's tile. */
    TilePosition tilePosition{};

    /** If this is a Floor or Object tile phantom, this is the phantom's offset
        relative to tilePosition.
        Note: Terrain and Wall phantoms shouldn't be given offsets, 
              they're already handled by WorldSpriteSorter. */
    TileOffset tileOffset{};

    /** The phantom's tile layer type. If this != None, this is a tile phantom.
        If this == None, this is an entity phantom. */
    TileLayer::Type layerType{TileLayer::Type::None};

    /** If layerType == Wall, this is the type of wall. */
    Wall::Type wallType{Wall::Type::None};

    /** If this is an entity phantom (layerType == None), this is the phantom's
        world position. */
    Position position{};

    /** The graphic set of the phantom sprite to add. */
    const GraphicSet* graphicSet{nullptr};

    /** The graphic value of the phantom sprite to add. */
    Uint8 graphicValue{};
};

} // End namespace Client
} // End namespace AM
