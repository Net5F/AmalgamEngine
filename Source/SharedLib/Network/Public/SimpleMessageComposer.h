#pragma once

#include "BinaryBuffer.h"
#include "ByteTools.h"
#include "NetworkDefs.h"
#include "Serialize.h"
#include "asio/error.hpp"
#include "asio/error_code.hpp"
#include <concepts>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace AM
{

/**
 * Incoming message composer for our simple "type + size, no batching" protocol.
 *
 * Note: Since deserialization requires extra context, we defer it to the 
 *       MessageProcessor classes.
 */
template<typename MessageType>
class SimpleMessageComposer
{
public:
    static_assert(std::is_enum_v<MessageType>,
                  "SimpleMessageComposer requires an enum message type.");
    static_assert(
        sizeof(MessageType) == sizeof(Uint8),
        "SimpleMessageComposer message enums must have a one-byte base.");

    SimpleMessageComposer(std::size_t inMaxPayloadSize)
    : state{ReadState::Header}
    , maxPayloadSize{inMaxPayloadSize}
    , pendingType{}
    , pendingPayloadSize{0}
    {
        if (maxPayloadSize > std::numeric_limits<Uint16>::max()) {
            LOG_FATAL("Payload limit does not fit in Uint16.");
        }
    }

    /**
     * Returns the exact byte count that addBytes() expects next.
     *
     * Used by the parent Connection to know how many bytes to wait for before 
     * passing them to this composer.
     */
    std::size_t nextReadSize() const
    {
        if (state == ReadState::Header) {
            return MESSAGE_HEADER_SIZE;
        }
        return pendingPayloadSize;
    }

    struct ComposedMessage {
        MessageType type{};
        BinaryBuffer payload{};
    };
    struct ComposeResult {
        asio::error_code error{};
        bool frameComplete{false};
        /** If frameComplete == true, this is the message that the frame 
            contained. */
        ComposedMessage message{};
    };

    /**
     * Consumes the exact byte buffer requested by nextReadSize().
     */
    ComposeResult addBytes(BinaryBuffer buffer)
    {
        if (buffer.size() != nextReadSize()) {
            return makeError(
                asio::error::make_error_code(asio::error::invalid_argument));
        }

        // If we were waiting for a header, track the header info and return.
        if (state == ReadState::Header) {
            pendingType = static_cast<MessageType>(
                buffer[MessageHeaderIndex::MessageType]);
            pendingPayloadSize
                = ByteTools::read16(buffer.data() + MessageHeaderIndex::Size);

            if (pendingPayloadSize > maxPayloadSize) {
                return makeError(
                    asio::error::make_error_code(asio::error::message_size));
            }

            // This was a 0-byte message, return it and reset.
            if (pendingPayloadSize == 0) {
                ComposeResult result{};
                result.frameComplete = true;
                result.message = {pendingType, {}};
                reset();
                return result;
            }

            state = ReadState::Payload;
            return {};
        }

        // We were waiting for a message payload. Process it and reset.
        ComposeResult result{};
        result.frameComplete = true;
        result.message = {pendingType, std::move(buffer)};
        reset();
        return result;
    }

    void reset()
    {
        state = ReadState::Header;
        pendingType = {};
        pendingPayloadSize = 0;
    }

private:
    enum class ReadState { Header, Payload };

    ComposeResult makeError(const asio::error_code& error)
    {
        reset();
        ComposeResult result{};
        result.error = error;
        return result;
    }

    std::size_t maxPayloadSize;

    /** The next part of the message frame that we expect to read. */
    ReadState state;

    /** If state==Payload, this is the type that we expect the next message to 
        be (based on the received header). */
    MessageType pendingType;

    /** If state==Payload, this is the size that we expect the next payload to 
        be (based on the received header). */
    std::size_t pendingPayloadSize;
};

} // namespace AM
