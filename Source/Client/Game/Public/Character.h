#ifndef CHARACTER_H
#define CHARACTER_H

#include <NWStructs.h>
#include <SDL_stdinc.h>

namespace NW
{

/**
 * \class Character
 * 
 * \brief Parent for PCs and NPCs. Has generic character members.
 * 
 *
 * \author Michael Puskas
 *
 * Created on: Feb 6, 2019
 *
 */
class Character
{
private:
    /* A local copy of where the character is in the world map.
       Should always match the character's position in the
       CharacterManager's array. */
    RCVec rcPosition;

    /* A multiplicative modifier used when moving the character. */
    Uint8 movementSpeed;

public:
    const RCVec& GetRCPosition() const;

    void SetRCPosition(const RCVec& InRCPosition);


    Character(RCVec InRCPosition, Uint32 InMovementSpeed);

    /**
     * Virtual destructor implemented only to remove compiler warning.
     * All members follow RAII, nothing to clean up.
     */
    virtual ~Character() { };

};

} /* End namespace NW */

#endif /* End CHARACTER_H */
