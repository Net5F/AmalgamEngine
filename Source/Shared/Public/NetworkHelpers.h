#ifndef NETWORKHELPERS_H
#define NETWORKHELPERS_H

#include "NetworkDefs.h"
#include <memory>

namespace AM
{

class Peer;

/**
 * Shared static functions for network tasks.
 */
class NetworkHelpers
{
public:
    /**
     * Allocates and fills a dynamic buffer with message data.
     *
     * The first 2 bytes of the buffer will contain the message size as a Uint16,
     * the rest will have the data at messageBuffer copied into it.
     *
     * For use with the Message type in our flatbuffer scheme. We aim to send the whole
     * Message + size with one send call, so it's convenient to have it all in
     * one buffer.
     */
    static BinaryBufferSharedPtr constructMessage(std::size_t size, Uint8* messageBuffer);
};

} // namespace AM

#endif /* NETWORKHELPERS_H */
