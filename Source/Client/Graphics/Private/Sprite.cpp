#include <Sprite.h>
#include <iostream>

const SDL2pp::Rect& NW::Sprite::GetSourceRect() const
{
    return sourceRect;
}

const SDL2pp::Texture& NW::Sprite::GetSourceTexture() const
{
    return sourceTexture;
}

NW::Sprite::Sprite (const SDL2pp::Texture& InSourceTexture,
                    const SDL2pp::Rect InSourceRect) :
                    sourceTexture(InSourceTexture),
                    sourceRect(InSourceRect)
{
}
