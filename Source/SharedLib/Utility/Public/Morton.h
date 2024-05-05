#pragma once

#include <SDL_stdinc.h>

/**
 * Functions for calculating morton codes.
 * 
 * Note: If we ever want to take advantage of the faster instruction set 
 *       support, or want to use more than 1 function from libmorton, we 
 *       should pull it in as a submodule.
 *
 * Source: https://github.com/Forceflow/libmorton/tree/main
 */
namespace AM
{
class Morton
{
public:
    /**
     * Returns a morton code for values of up to 16 using a lookup table.
     * Note that this is for values up to numeric 16, not 16 bits.
     */
    constexpr Uint16 m2D_lookup_16x16(Uint8 x, Uint8 y);

    /**
     * Returns a morton code for values of up to 16 bits.
     */
    constexpr Uint32 m2D_e_magicbits_combined(Uint16 x, Uint16 y);
};

} // End namespace AM
