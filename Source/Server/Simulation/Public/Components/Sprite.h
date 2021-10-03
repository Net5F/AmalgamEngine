#pragma once

#include "BoundingBox.h"
#include <SDL2/SDL_rect.h>
#include <string>

namespace AM
{
namespace Server
{
/**
 * Represents all of the sprite data that the RenderSystem needs, except for
 * the world position.
 *
 * World position should be read from an associated Position component.
 */
struct Sprite {
public:
    /** Unique display name, shown in the UI.  */
    std::string displayName{""};

    /** The sprite's unique string ID. Derived from displayName by replacing
        spaces with underscores and making everything lowercase.
        This ID will be consistent, and can be used for persistent state. */
    std::string stringID{""};

    /** The sprite's unique numeric identifier.
        This value can be used safely at runtime, but shouldn't be used for
        persistent state since it may change when SpriteData.json is
        modified. */
    int numericID{0};

    /** True if this sprite has a bounding box, else false.
        Things like floors and carpets share bounds with their tile, so they
        don't need a separate bounding box. */
    bool hasBoundingBox{true};

    /** Model-space bounding box. Defines the sprite's 3D volume. */
    BoundingBox modelBounds{0, 0, 0, 0, 0, 0};
};

} // namespace Server
} // namespace AM
