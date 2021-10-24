#pragma once

#include <SDL2/SDL_stdinc.h>
#include "bitsery/adapter/buffer.h"

namespace AM
{
/**
 * A struct that wraps a Uint8*, allowing us to use it as a buffer type for
 * serialization.
 *
 * Only used internally during serialization. Don't re-use this elsewhere.
 *
 * Note: Deserialization works fine with a raw Uint8*, but serialization
 *       requires that we define this type.
 */
struct SerializeBuffer {
    Uint8* buffer{nullptr};
    std::size_t size{0};

    Uint8* begin() const { return buffer; }

    Uint8* end() const { return (buffer + size); }
};

} // End namespace AM

/**
 * Required by Bitsery to use SerializeBuffer as a proper container.
 */
template<>
struct bitsery::traits::ContainerTraits<AM::SerializeBuffer> {
    using TValue = Uint8;
    static constexpr bool isResizable = false;
    static constexpr bool isContiguous = true;

    static void resize(AM::SerializeBuffer&, std::size_t) {}

    static std::size_t size(const AM::SerializeBuffer& buffer)
    {
        return buffer.size;
    }
};

/**
 * Required by Bitsery to use the above container as a buffer.
 */
template<>
struct bitsery::traits::BufferAdapterTraits<AM::SerializeBuffer> {
    using TIterator = Uint8*;
    using TConstIterator = const Uint8*;
    using TValue = Uint8;
};
