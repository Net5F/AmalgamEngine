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
     * Note that this is for values up to the number 16, not 16 bits.
     */
    static Uint8 m2D_lookup_16x16(Uint8 x, Uint8 y);

    struct Result2D
    {
        Uint8 x{};
        Uint8 y{};
    };
    /**
     * Returns the x, y values for a given morton code in the 16x16 value space.
     */
    static Result2D m2D_reverse_lookup_16x16(Uint8 code);

    /**
     * Returns a morton code for values of up to 16 bits.
     */
    static Uint32 m2D_e_magicbits_combined(Uint16 x, Uint16 y);
};

} // End namespace AM
