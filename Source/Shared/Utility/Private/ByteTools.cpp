#include "ByteTools.h"
#include "Peer.h"
#include "Log.h"
#include <SDL2/SDL_endian.h>
#include "zlib-ng.h"

// If the system has data access alignment restrictions, our casting may fail.
#if defined(sparc) || defined(mips) || defined(__arm__)
#error                                                                         \
    "ByteTools does not yet support systems with data access alignment restrictions."
#endif

// Uint8 should simply be an alias for unsigned char, but it isn't required
// to be by the standard. This check makes sure there's isn't any extra weird
// stuff in the Uint8 implementation on this platform.
// See: https://stackoverflow.com/a/26746305/4258629
static_assert(std::is_same<Uint8, unsigned char>::value,
              "We require Uint8 to be implemented as unsigned char.");

namespace AM
{
Uint16 ByteTools::read16(const Uint8* buffer)
{
    return SDL_SwapLE16(*reinterpret_cast<const Uint16*>(buffer));
}

Uint32 ByteTools::read32(const Uint8* buffer)
{
    return SDL_SwapLE32(*reinterpret_cast<const Uint32*>(buffer));
}

void ByteTools::write16(Uint16 value, Uint8* buffer)
{
    // Note: SwapLE16 does nothing on little endian systems.
    *reinterpret_cast<Uint16*>(buffer) = SDL_SwapLE16(value);
}

void ByteTools::write32(Uint32 value, Uint8* buffer)
{
    *reinterpret_cast<Uint32*>(buffer) = SDL_SwapLE32(value);
}

std::size_t ByteTools::compress(const Uint8* sourceBuffer,
                                std::size_t sourceLength, Uint8* destBuffer,
                                std::size_t destLength)
{
    // Compress the data.
    std::size_t destLengthReturn{destLength};
    int32_t result = zng_compress2(destBuffer, &destLengthReturn, sourceBuffer,
                                   sourceLength, COMPRESSION_LEVEL);

    // Check for errors.
    if (result == Z_MEM_ERROR) {
        LOG_ERROR("Ran out of memory while compressing.");
    }
    else if (result == Z_BUF_ERROR) {
        LOG_ERROR("Ran out of room in destBuffer while compressing.");
    }

    // Return the compressed data length.
    return destLengthReturn;
}

std::size_t ByteTools::uncompress(const Uint8* sourceBuffer,
                                  std::size_t sourceLength, Uint8* destBuffer,
                                  std::size_t destLength)
{
    // Compress the data.
    std::size_t destLengthReturn{destLength};
    int32_t result = zng_uncompress(destBuffer, &destLengthReturn, sourceBuffer,
                                    sourceLength);

    // Check for errors.
    if (result == Z_MEM_ERROR) {
        LOG_ERROR("Ran out of memory while uncompressing.");
    }
    else if (result == Z_BUF_ERROR) {
        LOG_ERROR("Ran out of room in destBuffer while uncompressing.");
    }
    else if (result == Z_DATA_ERROR) {
        LOG_ERROR("Data corrupted or incomplete while uncompressing.");
    }

    // Return the uncompressed data length.
    return destLengthReturn;
}

} // End namespace AM
