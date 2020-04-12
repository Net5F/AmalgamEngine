#include <Tile.h>

const std::string& NW::Tile::GetName () const
{
    return name;
}

bool NW::Tile::GetIsWalkable () const
{
    return bIsWalkable;
}

NW::Tile::Tile(const std::string& InName, bool bInIsWalkable
               , const SDL2pp::Texture& InSourceTexture
               , const SDL2pp::Rect InSourceRect) :
        name(InName), bIsWalkable(bInIsWalkable)
        , Sprite(InSourceTexture, InSourceRect)
{
}
