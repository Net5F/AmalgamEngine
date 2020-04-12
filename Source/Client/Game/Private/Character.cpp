#include <Character.h>

const NW::RCVec& NW::Character::GetRCPosition() const
{
    return rcPosition;
}

NW::Character::Character(RCVec InRCPosition, Uint32 InMovementSpeed) :
        rcPosition{InRCPosition},
        movementSpeed(InMovementSpeed)
{
}
