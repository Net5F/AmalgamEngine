#pragma once

#include <SDL2/SDL_stdinc.h>

/**
 * This file contains helper functions for writing to and reading from byte
 * buffers.
 *
 * Amalgam uses little endian ordering for network and file data.
 */
namespace AM
{

class ByteTools {
public:
    /**
     * Writes the given 16 byte value to the given buffer address.
     */
    static void write16(Uint16 value, Uint8* buffer);

    /**
     * Writes the given 32 byte value to the given buffer address.
     */
    static void write32(Uint32 value, Uint8* buffer);

    /**
     * Returns the 16 byte value at the given buffer address.
     */
    static Uint16 read16(const Uint8* buffer);

    /**
     * Returns the 32 byte value at the given buffer address.
     */
    static Uint32 read32(const Uint8* buffer);
};

} // End namespace AM
