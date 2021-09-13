#pragma once

#include "NetworkDefs.h"
#include "BinaryBuffer.h"
#include "Log.h"

namespace AM
{
/**
 * This class provides message-related static helper functions for things
 * like serialization and error checking.
 */
class MessageTools
{
public:
    /**
     * Writes the message header (message type and size) into the given buffer.
     * Also shrinks the buffer to fit the content, if it's over-sized.
     *
     * The first byte at startIndex will contain the message type as a Uint8.
     * The next 2 bytes will contain the message size as a Uint16.
     * The rest will be untouched.
     *
     * @param startIndex  Used to leave room at the front of the message to
     *                    later be filled. The client uses this since it
     *                    writes the client message header into the same
     *                    buffer. The server doesn't since it batches.
     */
    static void fillMessageHeader(MessageType type, std::size_t messageSize,
                                  const BinaryBufferSharedPtr& messageBuffer,
                                  unsigned int startIndex);
};

} // End namespace AM
