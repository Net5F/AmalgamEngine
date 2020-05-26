#ifndef NETWORKDEFS_H
#define NETWORKDEFS_H

#include <SDL_stdinc.h>
#include <memory>
#include <vector>

/**
 * This file contains shared network definitions that should be
 * consistent between the server and client.
 */
namespace AM
{

/** Dynamically allocated, portable buffers for messages. */
typedef std::unique_ptr<std::vector<Uint8>> BinaryBufferPtr;
typedef std::shared_ptr<std::vector<Uint8>> BinaryBufferSharedPtr;


/** Used for indexing into the parts of a server header. */
enum ServerHeaderIndex : Uint8 {
    // Sint8
    TickOffsetAdjustment = 0,
    // Uint8
    MessageCount = 1
};
/** The size of a server header in bytes. */
static constexpr unsigned int SERVER_HEADER_SIZE = 2;

/** Used for indexing into the size or payload of a received message. */
enum MessageIndex : Uint8 {
    // Uint16
    Size = 0,
    // Uint8[]
    MessageStart = 2
};

} /* End namespace AM */

#endif /* End NETWORKDEFS_H */
