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
    /** This sprite's unique key. Will be saved in the json as a string, but
        is kept as a hashed_string here to make comparisons faster. */
    entt::hashed_string key;

    /** UV position and size in texture. */
    SDL2pp::Rect textureExtent{0, 0, 0, 0};

    /** Model-space bounding box. Defines the sprite's 3D volume. */
    BoundingBox modelBounds{0, 0, 0, 0, 0, 0};
};

} // namespace SpriteEditor
} // namespace AM
