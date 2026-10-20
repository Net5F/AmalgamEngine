#pragma once

#include "BinaryBuffer.h"
#include "asio/buffer.hpp"
#include "asio/error_code.hpp"
#include <SDL3/SDL_stdinc.h>
#include <cstddef>
#include <span>
#include <vector>

namespace AM
{
namespace Client
{

/**
 * Incoming batch composer for the WorldServer -> Client protocol.
 *
 * The server sends a header, followed by a (potentially compressed) batch of
 * messages. This class owns the receive buffers: the caller reads directly
 * into nextReadBuffer(), then calls onBytesRead().
 *
 * Note: Since deserialization requires extra context, we defer it to
 *       MessageProcessor.
 */
class WorldBatchComposer
{
public:
    struct ReceivedMessage {
        Uint8 type{0};
        std::span<Uint8> payload{};
    };

    /**
     * A received, decompressed, and validated batch.
     *
     * Note: The spans point into this composer's buffers, and are only valid
     *       until the next read.
     */
    struct ReceivedBatch {
        /** The adjustment that the server wants us to make. */
        Sint8 tickAdjustment{0};
        /** The iteration of tick offset adjustment that the server is on. */
        Uint8 adjustmentIteration{0};
        /** The number of bytes that this batch took on the wire, including
            the header. */
        std::size_t wireSize{0};
        std::span<const ReceivedMessage> messages{};
    };

    struct ComposeResult {
        asio::error_code error{};
        bool batchComplete{false};
        /** If batchComplete == true, this is the received batch. */
        ReceivedBatch batch{};
    };

    WorldBatchComposer();

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
    enum class ReadState { Header, Batch };

    ComposeResult parseHeader();

    ComposeResult parseBatch();

    /**
     * Splits the given batch into messages and returns it.
     */
    ComposeResult completeBatch(std::span<Uint8> batch);

    /**
     * Fills messages with the messages in the given batch.
     * Returns an error if any message isn't fully contained in the batch.
     */
    asio::error_code splitMessages(std::span<Uint8> batch);

    ComposeResult makeError(const asio::error_code& error);

    /** The next part of the batch that we expect to read. */
    ReadState state;

    /** Holds a received server header. */
    BinaryBuffer headerBuffer;
    /** Holds a received batch. Sized to fit any batch the header can
        describe. */
    BinaryBuffer batchBuffer;
    /** If a batch is compressed, it's decompressed into this buffer. */
    BinaryBuffer decompressedBuffer;

    /** If state==Batch, these are the values from the received header. */
    Sint8 pendingTickAdjustment;
    Uint8 pendingAdjustmentIteration;
    std::size_t pendingBatchSize;
    bool pendingBatchIsCompressed;

    /** The messages in the most recently completed batch. Reused to avoid
        allocating for each batch. */
    std::vector<ReceivedMessage> messages;
};

} // namespace Client
} // namespace AM
