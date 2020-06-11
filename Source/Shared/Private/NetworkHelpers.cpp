#include "NetworkHelpers.h"
#include "SDL_net.h"
#include "SDL_stdinc.h"
#include <algorithm>
#include "Peer.h"
#include "Debug.h"

namespace AM
{

BinaryBufferSharedPtr AM::NetworkHelpers::constructMessage(std::size_t size,
                                                           Uint8* messageBuffer)
{
    if ((sizeof(Uint16) + size) > Peer::MAX_MESSAGE_SIZE) {
        DebugError("Tried to send a too-large message. Size: %u, max: %u", size,
            Peer::MAX_MESSAGE_SIZE);
    }

    // Allocate a buffer that can hold the Uint16 size bytes and the message payload.
    BinaryBufferSharedPtr dynamicBuffer = std::make_shared<std::vector<Uint8>>(
        sizeof(Uint16) + size);

    // Copy the size into the buffer.
    _SDLNet_Write16(size, dynamicBuffer->data());

    // Copy the message into the buffer.
    std::copy(messageBuffer, messageBuffer + size,
        dynamicBuffer->data() + sizeof(Uint16));

    return dynamicBuffer;
}

} // namespace AM
