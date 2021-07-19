#pragma once

#include "BoundingBox.h"
#include "entt/core/hashed_string.hpp"
#include <SDL_rect.h>
#include <SDL_stdinc.h>

namespace AM
{
namespace SpriteEditor
{

class SpriteSheet;

/**
 * Holds the static data for a single sprite.
 */
struct SpriteStaticData
{
public:
    /** The sprite sheet that this sprite is from. */
    SpriteSheet& parentSpriteSheet;

    /** Display name, shown in the sprite panel. */
    std::string displayName{""};

    /** Unique ID, this is what the map uses to reference sprites. */
    Uint16 id{0};

    /** UV position and size in texture. */
    SDL_Rect textureExtent{0, 0, 0, 0};

    /** How much this sprite should be offset in the Y direction to line up
        with its tile. Used to support tall tiles for the iso depth effect. */
    int yOffset{0};

    /** Model-space bounding box. Defines the sprite's 3D volume. */
    BoundingBox modelBounds{0, 0, 0, 0, 0, 0};
};

} // namespace SpriteEditor
} // namespace AM
