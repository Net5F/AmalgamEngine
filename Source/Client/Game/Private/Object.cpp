#include <Object.h>

int NW::Object::GetXPosition() const
{
    return xPosition;
}

int NW::Object::GetYPosition() const
{
    return yPosition;
}

bool NW::Object::GetIsWalkable() const
{
    return bIsWalkable;
}

NW::Object::Object(int InIdentifier, int InXPosition
                   , int InYPosition, bool bInIsWalkable
                   , const SDL2pp::Texture& InSourceTexture
                   , const SDL2pp::Rect InSourceRect) :
        identifier(InIdentifier)
        , xPosition(InXPosition)
        , yPosition(InYPosition)
        , bIsWalkable(bInIsWalkable)
        , Sprite(InSourceTexture, InSourceRect)
{
}
