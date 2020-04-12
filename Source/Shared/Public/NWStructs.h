#ifndef NWSTRUCTS_H
#define NWSTRUCTS_H

#include <SDL_stdinc.h>

namespace NW
{

/**
 * This file contains generic structs for use in all modules.
 * 
 *
 * \author Michael Puskas
 *
 * Created on: Feb 9, 2019
 *
 */

/**
 * An (x, y) value.
 * Used for representing pixel-based things like
 * screen position or screen size.
 */
struct XYVec
{
    Sint32 xVal;

    Sint32 yVal;
};

/**
 * A (row, column) value.
 * Used for representing tile-based things like
 * map position or size of the world map.
 */
struct RCVec
{
    Sint32 rowVal;

    Sint32 columnVal;
};

} /* End namespace NW */

#endif /* End NWSTRUCTS_H */
