#ifndef AMSTRUCTS_H
#define AMSTRUCTS_H

#include <SDL_stdinc.h>

namespace AM
{

/**
 * This file contains generic structs for use in all modules.
 */

/**
 * An (x, y) value.
 */
struct XYVec
{
    Sint32 x;

    Sint32 y;
};

} /* End namespace AM */

#endif /* End AMSTRUCTS_H */
