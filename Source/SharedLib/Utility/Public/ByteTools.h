#pragma once

#include <SDL_stdinc.h>

/**
 * Static functions for writing to and reading from byte buffers.
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
    /**
     * Returns the size that a destination buffer should be for the efficient
     * compression of source data of the given length.
     *
     * Our compression may work if the buffer is smaller than this value, but
     * it may fail and will at least run more slowly.
     */
    static std::size_t compressBound(std::size_t sourceLength);

    /**
     * Compresses data.
     *
     * @param sourceBuffer  A buffer containing the data to compress.
     * @param sourceLength  The length of the source data.
     * @param destBuffer  The buffer to write the compressed data to.
     * @param destLength  The length of the destination buffer. See
     *                    compressBound() for more info.
     * @return The length of the compressed data.
     */
    static std::size_t compress(const Uint8* sourceBuffer,
                                std::size_t sourceLength, Uint8* destBuffer,
                                std::size_t destLength);

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
    static std::size_t decompress(const Uint8* sourceBuffer,
                                  std::size_t sourceLength, Uint8* destBuffer,
                                  std::size_t destLength);
};

} // End namespace AM
