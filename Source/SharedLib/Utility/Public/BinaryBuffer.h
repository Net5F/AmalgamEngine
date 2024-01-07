#pragma once

#include <SDL_stdinc.h>
#include <vector>
#include <memory>

namespace AM
{
/** Dynamically allocated, portable buffers for bytes. */
using BinaryBuffer = std::vector<Uint8>;
using BinaryBufferPtr = std::unique_ptr<BinaryBuffer>;
using BinaryBufferSharedPtr = std::shared_ptr<BinaryBuffer>;

} // End namespace AM
