#include "ByteTools.h"
#include "Peer.h"
#include "Log.h"
#include "AMAssert.h"
#include <SDL_endian.h>
#include "lz4.h"

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

std::size_t ByteTools::compressBound(std::size_t sourceLength)
{
    return LZ4_compressBound(static_cast<int>(sourceLength));
}

std::size_t ByteTools::compress(const Uint8* sourceBuffer,
                                std::size_t sourceLength, Uint8* destBuffer,
                                std::size_t destLength)
{
    // Check that destBuffer is large enough for efficient compression.
    AM_ASSERT((static_cast<int>(destLength) < compressBound(sourceLength)),
              "Please increase destLength to at least %uB.",
              compressBound(sourceLength));

    // Compress the data.
    int compressedLength{LZ4_compress_default(
        reinterpret_cast<const char*>(sourceBuffer),
        reinterpret_cast<char*>(destBuffer), static_cast<int>(sourceLength),
        static_cast<int>(destLength))};

    // Check for errors.
    if (compressedLength <= 0) {
        LOG_FATAL("Error during compression.");
    }

    return compressedLength;
}

std::size_t ByteTools::decompress(const Uint8* sourceBuffer,
                                  std::size_t sourceLength, Uint8* destBuffer,
                                  std::size_t destLength)
{
    // Decompress the data.
    int decompressedLength{LZ4_decompress_safe(
        reinterpret_cast<const char*>(sourceBuffer),
        reinterpret_cast<char*>(destBuffer), static_cast<int>(sourceLength),
        static_cast<int>(destLength))};

    // Check for errors.
    if (decompressedLength < 0) {
        LOG_FATAL("Error during decompression.");
    }

    return decompressedLength;
}

} // End namespace AM
