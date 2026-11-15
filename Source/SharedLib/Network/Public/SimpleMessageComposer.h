#pragma once

#include "BinaryBuffer.h"
#include "ByteTools.h"
#include "NetworkDefs.h"
#include "Log.h"
#include "asio/buffer.hpp"
#include "asio/error.hpp"
#include "asio/error_code.hpp"
#include <algorithm>
#include <array>
#include <cstddef>
#include <limits>
#include <span>
#include <type_traits>

namespace AM
{

/**
 * Incoming message composer for our simple "type + size, no batching" protocol.
 *
 * This class owns the receive buffers: the caller reads directly into
 * getNextReadBuffer(), then calls processReadBytes().
 *
 * Each frame's first byte is read separately from the rest of its header. This
 * lets the caller use isFrameInProgress() to distinguish an idle connection
 * from a partially received frame (e.g. to detect slowloris-style attacks).
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
    static_assert(MESSAGE_HEADER_SIZE > 1,
                  "SimpleMessageComposer reads the header's first byte "
                  "separately, so the header must be more than 1 byte.");

    SimpleMessageComposer(std::size_t inMaxPayloadSize)
    : state{ReadState::FrameStart}
    , maxPayloadSize{inMaxPayloadSize}
    , headerBuffer{}
    , payloadBuffer{}
    , pendingType{}
    , pendingPayloadSize{0}
    {
        if (maxPayloadSize > std::numeric_limits<Uint16>::max()) {
            LOG_FATAL("Payload limit does not fit in Uint16.");
        }

        payloadBuffer.resize(
            std::min(maxPayloadSize, INITIAL_PAYLOAD_CAPACITY));
    }

    ~SimpleMessageComposer()
    {
        // Wipe anything we received, since it may hold secrets (e.g. if we
        // disconnected partway through a message).
        ByteTools::wipe(headerBuffer.data(), headerBuffer.size());
        ByteTools::wipe(payloadBuffer.data(), payloadBuffer.size());
    }

    struct ComposedMessage {
        MessageType type{};
        /** Note: Only valid until the next read. */
        std::span<Uint8> payload{};
    };
    struct ComposeResult {
        asio::error_code error{};
        /** True if these bytes were the first of a new frame. */
        bool frameStarted{false};
        bool frameComplete{false};
        /** If frameComplete == true, this is the message that the frame
            contained. */
        ComposedMessage message{};
    };

    /**
     * Returns the buffer that the next read should exactly fill.
     */
    asio::mutable_buffer getNextReadBuffer()
    {
        switch (state) {
            case ReadState::FrameStart:
                return asio::buffer(headerBuffer.data(), 1);
            case ReadState::HeaderRest:
                return asio::buffer(headerBuffer) + 1;
            case ReadState::Payload:
            default:
                return asio::buffer(payloadBuffer.data(), pendingPayloadSize);
        }
    }

    /**
     * Consumes the bytes that were read into getNextReadBuffer().
     */
    ComposeResult processReadBytes()
    {
        switch (state) {
            case ReadState::FrameStart: {
                state = ReadState::HeaderRest;
                ComposeResult result{};
                result.frameStarted = true;
                return result;
            }
            case ReadState::HeaderRest:
                return parseHeader();
            case ReadState::Payload:
            default:
                return completeMessage(
                    {payloadBuffer.data(), pendingPayloadSize});
        }
    }

    /**
     * Returns true if part of a frame has been received (i.e. we're partway
     * through a frame), else false.
     */
    bool isFrameInProgress() const { return state != ReadState::FrameStart; }

    void reset()
    {
        state = ReadState::FrameStart;
        pendingType = {};
        pendingPayloadSize = 0;
    }

private:
    enum class ReadState {
        /** Waiting for the first byte of a frame's header. */
        FrameStart,
        /** Waiting for the rest of the header. */
        HeaderRest,
        /** Waiting for the payload. */
        Payload
    };

    /** Most messages fit in this much space. If a larger message is
        received, we'll grow our buffer to the max payload size and keep it. */
    static constexpr std::size_t INITIAL_PAYLOAD_CAPACITY{ETHERNET_MTU};

    ComposeResult parseHeader()
    {
        pendingType = static_cast<MessageType>(
            headerBuffer[MessageHeaderIndex::MessageType]);
        pendingPayloadSize
            = ByteTools::read16(headerBuffer.data() + MessageHeaderIndex::Size);

        if (pendingPayloadSize > maxPayloadSize) {
            return makeError(
                asio::error::make_error_code(asio::error::message_size));
        }

        // If this is a 0-byte message, there's nothing more to read.
        if (pendingPayloadSize == 0) {
            return completeMessage({});
        }

        // If this message doesn't fit, grow straight to the max so that we
        // only grow once. Wipe first, since growing frees the old storage.
        if (pendingPayloadSize > payloadBuffer.size()) {
            ByteTools::wipe(payloadBuffer.data(), payloadBuffer.size());
            payloadBuffer.resize(maxPayloadSize);
        }

        state = ReadState::Payload;
        return {};
    }

    ComposeResult completeMessage(std::span<Uint8> payload)
    {
        ComposeResult result{};
        result.frameComplete = true;
        result.message = {pendingType, payload};
        reset();
        return result;
    }

    ComposeResult makeError(const asio::error_code& error)
    {
        reset();
        ComposeResult result{};
        result.error = error;
        return result;
    }

    /** The next part of the message frame that we expect to read. */
    ReadState state;

    std::size_t maxPayloadSize;

    /** Holds a received header. */
    std::array<Uint8, MESSAGE_HEADER_SIZE> headerBuffer;

    /** Holds a received payload.
        Note: This buffer's size is its allocated length. It only grows, so
              that wiping size() bytes always covers everything we received.
              Use pendingPayloadSize for the current message's length. */
    BinaryBuffer payloadBuffer;

    /** If state==Payload, this is the type that we expect the next message to
        be (based on the received header). */
    MessageType pendingType;

    /** If state==Payload, this is the size that we expect the next payload to
        be (based on the received header). */
    std::size_t pendingPayloadSize;
};

} // namespace AM
