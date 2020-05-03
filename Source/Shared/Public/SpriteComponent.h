#ifndef SPRITECOMPONENT_H_
#define SPRITECOMPONENT_H_

#include <memory>
#include <SDL2pp/SDL2pp.hh>

namespace AM
{

/**
 * This struct represents all of the sprite data that the RenderSystem needs, except for
 * the world position.
 *
 * World position should be pulled from an associated PositionComponent.
 */
struct SpriteComponent
{
public:
    SpriteComponent()
    : texturePtr(nullptr), posInTexture { 0, 0, 0, 0 }, width(0), height(0)
    {
    }

    // TODO: Switch to textureID and add a texture loader.
    /** A pointer to the texture that holds this sprite. */
    std::shared_ptr<SDL2pp::Texture> texturePtr;

    /** UV position and size in texture. */
    SDL2pp::Rect posInTexture;

    /** Width and height of sprite in the world. */
    int width;
    int height;
};

} /* End namespace AM */

#endif /* End SPRITECOMPONENT_H_ */
