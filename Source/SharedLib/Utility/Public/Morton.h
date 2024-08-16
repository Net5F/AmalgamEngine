#pragma once

#include <SDL_stdinc.h>

/**
 * Functions for calculating morton codes.
 * 
 * Note: We exclude the 3D morton codes because they aren't useful to us 
 *       (they waste a lot of space if your extent isn't a cube, and our Z 
 *       is almost always going to be a lot smaller than X/Y).
 *       If we ever care to use them, we can add them and change the naming 
 *       convention (e.g. encode2D32/decode2D32).
 */
namespace AM
{
class Morton
{
public:
    template<typename T>
    struct Result2D
    {
        T x{};
        T y{};
    };

    /**
     * Returns a morton code for values in the range [0, 15] using a lookup 
     * table.
     */
    static Uint8 encode16x16(Uint8 x, Uint8 y);

    /**
     * Returns the x, y values for a given morton code in the 16x16 value space.
     */
    static Result2D<Uint8> decode16x16(Uint8 code);

    /**
     * Returns a 32-bit morton code for values of up to 16 bits.
     */
    static Uint32 encode32(Uint16 x, Uint16 y);
    /**
     * Returns a 64-bit morton code for values of up to 32 bits.
     */
    static Uint64 encode64(Uint32 x, Uint32 y);

    /**
     * Returns 16-bit values for a morton code of up to 32 bits.
     */
    static Result2D<Uint16> decode32(Uint32 code);
    /**
     * Returns 32-bit values for a morton code of up to 64 bits.
     */
    static Result2D<Uint32> decode64(Uint64 code);
};

} // End namespace AM
