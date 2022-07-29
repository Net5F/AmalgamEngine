#pragma once

#include "BoundingBox.h"
#include <SDL_rect.h>
#include <string>
#include <vector>

namespace AM
{
namespace Server
{
/**
 * Holds the data for a single sprite from SpriteData.json.
 *
 * World position should be read from an associated Position component (if
 * this sprite is attached to an entity), or derived from an associated Tile.
 */
struct Sprite {
public:
    /** Unique display name, shown in the UI.  */
    std::string displayName{"Empty"};

    /** The sprite's unique string ID. Derived from displayName by replacing
        spaces with underscores and making everything lowercase.
        This ID will be consistent, and can be used for persistent state. */
    std::string stringID{"empty"};

    /** The sprite's unique numeric identifier.
        This value can be used safely at runtime, but shouldn't be used for
        persistent state since it may change when SpriteData.json is
        modified. */
    int numericID{-1};

    /** Model-space bounding boxes. Defines the sprite's 3D volume. */
    //std::vector<BoundingBox> modelBounds;
    bool hasBoundingBox{false};
    BoundingBox modelBounds{};
};

} // namespace Server
} // namespace AM
