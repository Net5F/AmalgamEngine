#ifndef SPRITE_H
#define SPRITE_H

#include <SDL2pp/SDL2pp.hh>

namespace NW
{

/**
 * \class Sprite
 *
 * \brief Wraps a texture with sprite features.
 *
 * This class is in charge of maintaining a clipping rect
 * and a shared pointer to its source texture.
 *
 *
 * \author Michael Puskas
 *
 * Created on: Jan 1, 2019
 *
 */
class Sprite
{
protected:
    /* Reference to the texture atlas that this sprite is from */
    const SDL2pp::Texture& sourceTexture;

    /* A rect storing the position of the sprite in UV texture space */
    const SDL2pp::Rect sourceRect;

public:
    const SDL2pp::Rect& GetSourceRect() const;
    const SDL2pp::Texture& GetSourceTexture() const;


    Sprite(const SDL2pp::Texture& InSourceTexture
           , const SDL2pp::Rect InSourceRect);
};

} /* End namespace NW */

#endif /* End SPRITE_H */
