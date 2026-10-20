#pragma once

#include "BinaryBuffer.h"
#include "ByteTools.h"
#include "NetworkDefs.h"
#include "Serialize.h"
#include "Log.h"
#include <type_traits>

namespace AM
{
namespace Client
{

/**
 * Outgoing message framer for the Client -> WorldServer protocol.
 *
 * Each message is sent individually (no batching), prefixed by a client header
 * and a message header.
 */
class WorldMessageFramer
{
public:
    /**
     * Serializes and frames a typed message for transport.
     *
     * @param message A structure that defines MESSAGE_TYPE and has an
     *                associated serialize() function.
     * @param adjustmentIteration The tick adjustment iteration that we're on.
     * @return A completed message, ready for sending. Empty if the message
     *         was too large.
     */
    template<typename Message>
    static BinaryBufferSharedPtr frameMessage(const Message& message,
                                              Uint8 adjustmentIteration);
};

template<typename Message>
BinaryBufferSharedPtr
    WorldMessageFramer::frameMessage(const Message& message,
                                     Uint8 adjustmentIteration)
{
    using DeclaredMessageType
        = std::remove_cv_t<decltype(Message::MESSAGE_TYPE)>;
    static_assert(std::is_enum_v<DeclaredMessageType>
                      && (sizeof(DeclaredMessageType) == sizeof(Uint8)),
                  "Message::MESSAGE_TYPE must be a one-byte enum.");

    // Check that the message isn't too big.
    // Note: We don't compress messages on this side, so we know the final
    //       message size at this point.
    std::size_t payloadSize{Serialize::measureSize(message)};
    std::size_t totalMessageSize{CLIENT_HEADER_SIZE + MESSAGE_HEADER_SIZE
                                 + payloadSize};
    if (totalMessageSize > CLIENT_MAX_MESSAGE_SIZE) {
        LOG_INFO("Tried to send a too-large message. Size: %u, max: %u",
                 totalMessageSize, CLIENT_MAX_MESSAGE_SIZE);
        return {};
    }

    // Serialize the message struct into a buffer, leaving room for the
    // headers.
    BinaryBufferSharedPtr buffer{
        std::make_shared<BinaryBuffer>(totalMessageSize)};
    std::size_t serializedSize{
        Serialize::toBuffer(buffer->data(), buffer->size(), message,
                            (CLIENT_HEADER_SIZE + MESSAGE_HEADER_SIZE))};
    if (serializedSize != payloadSize) {
        LOG_INFO("Measured and encoded message sizes do not match.");
        return {};
    }

    // Fill in the client header.
    buffer->at(ClientHeaderIndex::AdjustmentIteration) = adjustmentIteration;

    // Fill in the message header.
    buffer->at(CLIENT_HEADER_SIZE + MessageHeaderIndex::MessageType)
        = static_cast<Uint8>(Message::MESSAGE_TYPE);
    ByteTools::write16(
        static_cast<Uint16>(payloadSize),
        (buffer->data() + CLIENT_HEADER_SIZE + MessageHeaderIndex::Size));

    return buffer;
}

} // namespace Client
} // namespace AM
