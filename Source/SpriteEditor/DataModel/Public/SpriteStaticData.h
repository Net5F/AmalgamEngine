#pragma once

#include "BoundingBox.h"
#include "entt/core/hashed_string.hpp"
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_stdinc.h>

namespace AM
{
namespace SpriteEditor
{
class SpriteSheet;

/**
 * Holds the data for a single sprite.
 * Note: This class's weird name is to avoid a conflict with Shared's Sprite
 * Note: This data is "static" from the point of view of the eventual consumers
 *       (the client and server). In the SpriteEditor, this data is all
 *       mutable.
 *
 * Used as part of SpriteEditor's data model. Don't confuse this with the
 * actual Sprite concept in Shared, or the Client's SpriteStaticData.
 */
struct SpriteStaticData {
public:
    /** The sprite sheet that this sprite is from. */
    SpriteSheet& parentSpriteSheet;

    /** Display name, shown in the UI.  */
    std::string displayName{""};

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

} // namespace SpriteEditor
} // namespace AM
