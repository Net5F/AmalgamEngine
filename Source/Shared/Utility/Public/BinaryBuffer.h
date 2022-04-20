#pragma once

#include <SDL_stdinc.h>
#include <vector>
#include <memory>

namespace AM
{
/** Dynamically allocated, portable buffers for bytes. */
typedef std::vector<Uint8> BinaryBuffer;
typedef std::unique_ptr<BinaryBuffer> BinaryBufferPtr;
typedef std::shared_ptr<BinaryBuffer> BinaryBufferSharedPtr;

} // End namespace AM
