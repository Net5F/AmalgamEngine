#pragma once

#include "BoundingBox.h"
#include "NullSpriteID.h"
#include <string>

namespace AM
{
/**
 * Holds the data for a single sprite from ResourceData.json.
 *
 * World position should be read from an associated Position component (if
 * this sprite is attached to an entity), or derived from an associated Tile.
 */
struct Sprite {
    /** Unique display name, shown in the UI.  */
    std::string displayName{"Null"};

    /** The sprite's unique string ID. Derived from displayName by replacing
        spaces with underscores and making everything lowercase.
        This ID will be consistent, and can be used for persistent state. */
    std::string stringID{"null"};

    /** This sprite's unique numeric identifier.
        This value can be used safely at runtime, but shouldn't be used for
        persistent state since it may change when ResourceData.json is
        modified. */
    int numericID{NULL_SPRITE_ID};

    /** If true, this sprite's modelBounds will be used in collision checks.
        Most sprites will want collision enabled, but things like floors and 
        carpets usually don't need collision. */
    bool collisionEnabled{false};

    /** Model-space bounding box. Defines the sprite's 3D volume.
        Note: Tiles use these bounds, but dynamic entities use the bounds 
              defined by their Collision component. */
    BoundingBox modelBounds{0, 0, 0, 0, 0, 0};
};

} // namespace AM
