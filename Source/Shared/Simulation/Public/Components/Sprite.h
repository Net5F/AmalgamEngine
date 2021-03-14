#pragma once

#include "BoundingBox.h"
#include "SDL2pp/Texture.hh"
#include "SDL2pp/Rect.hh"
#include <memory>

namespace AM
{
/**
 * This struct represents all of the sprite data that the RenderSystem needs,
 * except for the world position.
 *
 * World position should be read from an associated Position component.
 */
struct Sprite {
public:
    // TODO: Switch to textureID and add a texture loader.
    /** A pointer to the texture that holds this sprite. */
    std::shared_ptr<SDL2pp::Texture> texturePtr{nullptr};

    /** UV position and size in texture. */
    SDL2pp::Rect texExtent{0, 0, 0, 0};

    /** Width and height of the sprite in screen space. */
    int width{0};
    int height{0};

    /** Model-space bounding box. Defines the sprite's 3D volume. */
    BoundingBox modelBounds{0, 0, 0, 0, 0, 0};

    /** World-space bounding box.
        Equal to modelBounds adjusted for the associated entity's current world
        Position. Used for depth order sorting. */
    BoundingBox worldBounds{0, 0, 0, 0, 0, 0};
};

} // namespace AM
