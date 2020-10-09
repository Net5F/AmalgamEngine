#include "MessageTools.h"
#include "Peer.h"
#include "SDL_net.h"

namespace AM
{
void MessageTools::fillMessageHeader(MessageType type, std::size_t messageSize,
                                     const BinaryBufferSharedPtr& messageBuffer,
                                     unsigned int startIndex)
{
    const unsigned int totalMessageSize
        = startIndex + MESSAGE_HEADER_SIZE + messageSize;
    if ((totalMessageSize > Peer::MAX_MESSAGE_SIZE)
        || (messageSize > UINT16_MAX)) {
        LOG_ERROR("Tried to send a too-large message. Size: %u, max: %u",
                  messageSize, Peer::MAX_MESSAGE_SIZE);
    }
    else if (totalMessageSize > messageBuffer->size()) {
        LOG_ERROR("Given buffer is too small. Size: %u, required: %u",
                  messageBuffer->size(), totalMessageSize);
    }

    // Copy the type into the buffer.
    messageBuffer->at(startIndex + MessageHeaderIndex::MessageType)
        = static_cast<Uint8>(type);

    // Copy the messageSize into the buffer.
    _SDLNet_Write16(messageSize, (messageBuffer->data() + startIndex
                                  + MessageHeaderIndex::Size));

    // Shrink the buffer to fit.
    messageBuffer->resize(totalMessageSize);
}

} // End namespace AM
