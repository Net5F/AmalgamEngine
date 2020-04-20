#ifndef AMSTRUCTS_H
#define AMSTRUCTS_H

#include <SDL_stdinc.h>

/**
 * This file contains shared definitions that should be
 * consistent between the server and client.
 */
namespace AM
{

static constexpr Uint32 MAX_ENTITIES = 100;

typedef uint32_t EntityID;

struct ComponentFlag
{
    enum FlagType
    {
        Position = 1 << 0,
        Movement = 1 << 1,
        Input = 1 << 2,
        Sprite = 1 << 3
    };
};

} /* End namespace AM */

#endif /* End AMSTRUCTS_H */
