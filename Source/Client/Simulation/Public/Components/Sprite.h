#pragma once

#include "AssetCache.h"
#include "BoundingBox.h"
#include <SDL2/SDL_rect.h>
#include <memory>

namespace AM
{
namespace Client
{
/**
 * Holds the data for a single sprite from SpriteData.json.
 *
 * World position should be read from an associated Position component (if
 * this sprite is attached to an entity), or derived from an associated Tile.
 */
struct Sprite {
public:
    /** The texture that contains this sprite. */
    TextureHandle texture{};

    /** Unique display name, shown in the UI.  */
    std::string displayName{""};

    /** The sprite's unique string ID. Derived from displayName by replacing
        spaces with underscores and making everything lowercase.
        This ID will be consistent, and can be used for persistent state. */
    std::string stringId{""};

    /** The sprite's unique numeric identifier.
        This value can be used safely at runtime, but shouldn't be used for
        persistent state since it may change when SpriteData.json is
        modified. */
    int numericId{0};

    /** UV position and size in texture. */
    SDL_Rect textureExtent{0, 0, 0, 0};

    /** How much this sprite should be offset in the Y direction to line up
        with its tile. Used to support tall tiles for the iso depth effect. */
    int yOffset{0};

    /** True if this sprite has a bounding box, else false.
        Things like floors and carpets share bounds with their tile, so they
        don't need a separate bounding box. */
    bool hasBoundingBox{true};

    /** Model-space bounding box. Defines the sprite's 3D volume. */
    BoundingBox modelBounds{0, 0, 0, 0, 0, 0};
};

} // namespace Client
} // namespace AM
