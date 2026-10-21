#pragma once

#include "BinaryBuffer.h"
#include "NetworkDefs.h"
#include "asio/buffer.hpp"
#include "asio/error_code.hpp"
#include <SDL3/SDL_stdinc.h>
#include <array>
#include <cstddef>
#include <span>

namespace AM
{
namespace WorldServer
{

/**
 * Incoming message composer for the Client -> WorldServer protocol.
 *
 * Clients send each message individually, prefixed by a client header and a
 * message header. This class owns the receive buffers: the caller reads
 * directly into nextReadBuffer(), then calls onBytesRead().
 *
 * Note: Since deserialization requires extra context, we defer it to
 *       MessageProcessor.
 */
class WorldClientMessageComposer
{
public:
    struct ReceivedMessage {
        /** The tick adjustment iteration that the client is on. */
        Uint8 adjustmentIteration{0};
        Uint8 type{0};
        /** Note: Only valid until the next read. */
        std::span<Uint8> payload{};
    };

    struct ComposeResult {
        asio::error_code error{};
        bool messageComplete{false};
        /** If messageComplete == true, this is the received message. */
        ReceivedMessage message{};
    };

    /**
     * @param inMaxPayloadSize The max size, in bytes, for a message payload.
     */
    WorldClientMessageComposer(std::size_t inMaxPayloadSize);

    /**
     * Sets the max size, in bytes, for a message payload.
     * Takes effect on the next received message.
     */
    void setMaxPayloadSize(std::size_t inMaxPayloadSize);

    /**
     * Returns the buffer that the next read should exactly fill.
     */
    asio::mutable_buffer nextReadBuffer();

    /**
     * Consumes the bytes that were read into nextReadBuffer().
     */
    ComposeResult onBytesRead();

    void reset();

private:
    enum class ReadState { Header, Payload };

    static constexpr std::size_t HEADER_SIZE{CLIENT_HEADER_SIZE
                                             + MESSAGE_HEADER_SIZE};

    /** Most messages fit in this much space. If a larger message is
        received, we'll grow our buffer to the max payload size and keep it
        (at most ~64KiB per client, see CLIENT_MAX_MESSAGE_SIZE). We expect
        larger messages to only come from devs. */
    static constexpr std::size_t INITIAL_PAYLOAD_CAPACITY{ETHERNET_MTU};

    ComposeResult parseHeader();

    ComposeResult completeMessage(std::span<Uint8> payload);

    ComposeResult makeError(const asio::error_code& error);

    /** The next part of the message that we expect to read. */
    ReadState state;

    std::size_t maxPayloadSize;

    /** Holds a received header. */
    std::array<Uint8, HEADER_SIZE> headerBuffer;

    /** Holds a received payload. */
    BinaryBuffer payloadBuffer;

    /** If state==Payload, these are the values from the received header. */
    Uint8 pendingAdjustmentIteration;
    Uint8 pendingType;
    std::size_t pendingPayloadSize;
};

} // End namespace WorldServer
} // End namespace AM
