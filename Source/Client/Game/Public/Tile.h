#ifndef TILE_H
#define TILE_H

#include <Sprite.h>
#include <SDL2pp/SDL2pp.hh>
#include <string>

namespace NW
{

/**
 * \class Tile
 * 
 * \brief Wraps the sprite class with tile-specific state data.
 * 
 * This class represents a type of tile. No two
 * instances of this class should be the same.
 *
 * The position of the tile is implicitly represented
 * by its position in the TileManager's TileMap.
 *
 *
 * \author Michael Puskas
 *
 * Created on: Jan 7, 2019
 *
 */
class Tile : public Sprite
{
private:
    /* TitleCase name of this type of tile */
    // TODO: Remove name? See if it's ever useful.
    const std::string name;

    /* Whether the player is allowed to move onto this tile or not */
    const bool bIsWalkable;

public:
    const std::string& GetName() const;
    bool GetIsWalkable() const;


    Tile(const std::string& InName, bool bInIsWalkable
         , const SDL2pp::Texture& InSourceTexture
         , const SDL2pp::Rect InSourceRect);

};

} /* End namespace NW */

#endif /* End TILE_H */
