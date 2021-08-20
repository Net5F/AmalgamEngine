#include "ByteTools.h"
#include <SDL2/SDL_endian.h>

// If the system has data access alignment restrictions, our casting may fail.
#if defined(sparc) || defined(mips) || defined(__arm__)
#error "ByteTools does not yet support systems with data access alignment restrictions."
#endif

// This check is just to make sure uint8_t doesn't have any weird stuff going
// on. See: https://stackoverflow.com/a/26746305/4258629
static_assert(std::is_same<Uint8, unsigned char>::value,
              "We require Uint8 to be implemented as unsigned char.");

namespace AM
{

void ByteTools::write16(Uint16 value, Uint8* buffer) {
    // Note: SwapLE16 does nothing on little endian systems.
    *reinterpret_cast<Uint16*>(buffer) = SDL_SwapLE16(value);
}

void ByteTools::write32(Uint32 value, Uint8* buffer) {
    *reinterpret_cast<Uint32*>(buffer) = SDL_SwapLE32(value);
}

Uint16 ByteTools::read16(const Uint8* buffer) {
    return SDL_SwapLE16(*reinterpret_cast<const Uint16*>(buffer));
}

Uint32 ByteTools::read32(const Uint8* buffer) {
    return SDL_SwapLE32(*reinterpret_cast<const Uint32*>(buffer));
}

} // End namespace AM
