#ifndef SPRITECOMPONENT_H_
#define SPRITECOMPONENT_H_

#include <memory>
#include <SDL2pp/SDL2pp.hh>

namespace AM
{

struct SpriteComponent
{
public:
    SpriteComponent()
    : texturePtr(nullptr), posInTexture { 0, 0, 0, 0 }, posInWorld { 0, 0, 0, 0 }
    {
    }

    // TODO: Switch to textureID and add a texture loader.
    /** A pointer to the texture that holds this sprite. */
    std::shared_ptr<SDL2pp::Texture> texturePtr;

    /** UV position and size in texture. */
    SDL2pp::Rect posInTexture;

    /** ST position and size in world. */
    SDL2pp::Rect posInWorld;
};

} /* End namespace AM */

#endif /* End SPRITECOMPONENT_H_ */
