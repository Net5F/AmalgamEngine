#ifndef OBJECT_H
#define OBJECT_H

#include <Sprite.h>
#include <SDL2pp/SDL2pp.hh>

namespace NW
{

/**
 * \class Object
 * 
 * \brief Wraps the sprite class with object-specific state data.
 * 
 * This class represents an object in the game world (a wall, table,
 * bush, etc).
 *
 * The position of the tile is implicitly represented
 * by its position in the ObjectManager's ObjectMap.
 * 
 *
 * \author Michael Puskas
 *
 * Created on: Jan 19, 2019
 *
 */
class Object : public Sprite
{
private:
    /* Unique identifier from the server's ID pool.
       Used for ID-based queries to the ObjectManager's
       object map. */
    int identifier;

    /*
       Position in relation to the game map (in pixels).

       Position is only useful for animations. The logical
       location of the object within the world is always
       implicitly represented by its location within the
       ObjectManager's ObjectMap.
     */
    int xPosition;
    int yPosition;

    /* Whether the player is allowed to walk on this object or not */
    const bool bIsWalkable;

public:
    bool GetIsWalkable() const;
    int GetXPosition() const;
    int GetYPosition() const;


    Object(int InIdentifier, int InXPosition
           , int InYPosition, bool bInIsWalkable
           , const SDL2pp::Texture& InSourceTexture
           , const SDL2pp::Rect InSourceRect);

};

} /* End namespace NW */

#endif /* End OBJECT_H */
