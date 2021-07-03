#pragma once

#include "BoundingBox.h"
#include "SDL2pp/Rect.hh"
#include "entt/core/hashed_string.hpp"

namespace AM
{
namespace SpriteEditor
{

/**
 * Holds the static data for a single sprite.
 */
struct SpriteStaticData
{
public:
    /** Display name, shown in the sprite panel. */
    std::string displayName;

    /** Unique ID, this is what the map uses to reference sprites. */
    Uint16 id;

    /** UV position and size in texture. */
    SDL2pp::Rect textureExtent{0, 0, 0, 0};

    /** Model-space bounding box. Defines the sprite's 3D volume. */
    BoundingBox modelBounds{0, 0, 0, 0, 0, 0};
};

} // namespace SpriteEditor
} // namespace AM
