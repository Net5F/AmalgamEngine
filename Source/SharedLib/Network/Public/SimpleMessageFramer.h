#pragma once

#include "BinaryBuffer.h"
#include "ByteTools.h"
#include "NetworkDefs.h"
#include "Serialize.h"
#include "Log.h"
#include <concepts>
#include <type_traits>

namespace AM
{

/**
 * Outgoing message framer for our simple "type + size, no batching" protocol.
 */
template<typename MessageType>
class SimpleMessageFramer
{
public:
    static_assert(std::is_enum_v<MessageType>,
                  "SimpleMessageFramer requires an enum message type.");
    static_assert(
        sizeof(MessageType) == sizeof(Uint8),
        "SimpleMessageFramer message enums must have a one-byte base.");

    SimpleMessageFramer(std::size_t inMaxPayloadSize)
    : maxPayloadSize{inMaxPayloadSize}
    {
        if (maxPayloadSize > std::numeric_limits<Uint16>::max()) {
            LOG_FATAL("Payload limit does not fit in Uint16.");
        }
    }

    /**
     * Serializes and frames a typed message for transport.
     * @return A completed message, ready for sending.
     */
    template<typename Message>
    BinaryBufferSharedPtr frameMessage(const Message& message) const
    {
        using DeclaredMessageType
            = std::remove_cv_t<decltype(Message::MESSAGE_TYPE)>;
        static_assert(
            std::same_as<DeclaredMessageType, MessageType>,
            "Message::MESSAGE_TYPE belongs to a different wire type space.");

        std::size_t payloadSize{Serialize::measureSize(message)};
        if (payloadSize > maxPayloadSize) {
            LOG_INFO("Encoded message exceeds codec limit.");
            return {};
        }

        BinaryBufferSharedPtr buffer{
            std::make_shared<BinaryBuffer>(MESSAGE_HEADER_SIZE + payloadSize)};
        std::size_t serializedSize{Serialize::toBuffer(
            buffer->data(), buffer->size(), message, MESSAGE_HEADER_SIZE)};
        if (serializedSize != payloadSize) {
            LOG_INFO("Measured and encoded message sizes do not match.");
            return {};
        }

        buffer->at(MessageHeaderIndex::MessageType)
            = static_cast<Uint8>(Message::MESSAGE_TYPE);
        ByteTools::write16(static_cast<Uint16>(payloadSize),
                           buffer->data() + MessageHeaderIndex::Size);
        return buffer;
    }

private:
    std::size_t maxPayloadSize;
};

} // namespace AM
