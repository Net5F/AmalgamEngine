#pragma once

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
    SDL2pp::Rect posInTexture{0, 0, 0, 0};

    /** Width and height of the sprite in world space. */
    int width{0};
    int height{0};
};

} // namespace AM
