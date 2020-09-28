#pragma once

#include "SDL_stdinc.h"
#include "InputComponent.h"
#include "PositionComponent.h"
#include "MovementComponent.h"

namespace AM
{

/**
 * This struct represents the data for a single entity.
 * Normally an entity's data lives in the ECS, so this is only really useful for messaging.
 */
struct Entity
{
    /** This entity's sim ID. */
    Uint32 id = 0;

    /** Bitflags showing which components this entity has. */
    Uint32 flags = 0;

    InputComponent& inputComponent;
    PositionComponent& positionComponent;
    MovementComponent& movementComponent;
};

template <typename S>
void serialize(S& serializer, Entity entity)
{
    serializer.value4b(entity.id);
    serializer.value4b(entity.flags);
    serializer.object(entity.inputComponent);
    serializer.object(entity.positionComponent);
    serializer.object(entity.movementComponent);
}

} // End namespace AM
