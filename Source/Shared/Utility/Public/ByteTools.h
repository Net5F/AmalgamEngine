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
class ByteTools
{
public:
    //-------------------------------------------------------------------------
    // Byte Reading/Writing
    //-------------------------------------------------------------------------
    /**
     * Returns the 16 byte value at the given buffer address.
     */
    static Uint16 read16(const Uint8* buffer);

    /**
     * Returns the 32 byte value at the given buffer address.
     */
    static Uint32 read32(const Uint8* buffer);

    /**
     * Writes the given 16 byte value to the given buffer address.
     */
    static void write16(Uint16 value, Uint8* buffer);

    /**
     * Writes the given 32 byte value to the given buffer address.
     */
    static void write32(Uint32 value, Uint8* buffer);

    //-------------------------------------------------------------------------
    // Compression
    //-------------------------------------------------------------------------
    /** The "level" value passed to zlib's compression function.
        Can be 0 - 9, with 0 being no compression, 1 being fastest, and 9 being
        most compressed. 6 is the default. */
    static constexpr int COMPRESSION_LEVEL = 6;

    /**
     * Compresses data.
     *
     * @param sourceBuffer  A buffer containing the data to compress.
     * @param sourceLength  The length of the source data.
     * @param destBuffer  The buffer to write the compressed data to. Must be
     *                    at least (sourceLength * 1.1 + 12) bytes long.
     * @param destLength  The length of the destination buffer.
     * @return The length of the compressed data.
     */
    static std::size_t compress(const Uint8* sourceBuffer, std::size_t sourceLength,
                                  Uint8* destBuffer, std::size_t destLength);

    /**
     * Decompresses data.
     *
     * @param sourceBuffer  A buffer containing the data to decompress.
     * @param sourceLength  The length of the source data.
     * @param destBuffer  The buffer to write the decompressed data to. Must be
     *                    long enough to hold the original data.
     * @param destLength  The length of the destination buffer.
     * @return The length of the decompressed data.
     */
    static std::size_t decompress(const Uint8* sourceBuffer, std::size_t sourceLength,
                                  Uint8* destBuffer, std::size_t destLength);
};

} // End namespace AM
