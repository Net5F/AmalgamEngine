#pragma once

#include "BoundingBox.h"
#include "EmptySpriteID.h"
#include <SDL_rect.h>
#include <string>

namespace AM
{
/**
 * Holds the data for a single sprite from SpriteData.json.
 *
 * World position should be read from an associated Position component (if
 * this sprite is attached to an entity), or derived from an associated Tile.
 */
struct Sprite {
public:
    /**
     * The sprite's display name and string ID fields are stored in flat 
     * vectors in the SpriteDataBase class, since they aren't commonly accessed
     * alongside the fields in this struct. 
     * See SpriteDataBase::getDisplayName() and SpriteDataBase::getStringID(), 
     * accessible through Client::SpriteData, Server::SpriteData.
     *
     * Additionally, the client stores the sprite's rendering-related data in 
     * the SpriteRenderData class. See Client::SpriteData::getRenderData().
     */

    /** The sprite's unique numeric identifier.
        This value can be used safely at runtime, but shouldn't be used for
        persistent state since it may change when SpriteData.json is
        modified. */
    int numericID{EMPTY_SPRITE_ID};

    /** True if this sprite has a bounding box, else false.
        Things like floors and carpets share bounds with their tile, so they
        don't need a separate bounding box. */
    bool hasBoundingBox{true};

    /** Model-space bounding box. Defines the sprite's 3D volume. */
    BoundingBox modelBounds{0, 0, 0, 0, 0, 0};
};

} // namespace AM
