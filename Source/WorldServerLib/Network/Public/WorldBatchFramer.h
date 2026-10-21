#pragma once

#include "BinaryBuffer.h"
#include "ByteTools.h"
#include "NetworkDefs.h"
#include "Serialize.h"
#include "SharedConfig.h"
#include <SDL3/SDL_stdinc.h>
#include <cstddef>
#include <memory>
#include <span>
#include <vector>

namespace AM
{
namespace WorldServer
{

/**
 * Outgoing batch framer for the WorldServer -> Client protocol.
 *
 * Messages are appended into a scratch buffer. finish() then compresses the
 * batch (if worthwhile) and copies it into a pooled buffer that's ready to be
 * sent.
 *
 * Only one batch can be built at a time, so a single framer can be shared by
 * all clients.
 */
class WorldBatchFramer
{
public:
    static_assert(SharedConfig::MAX_BATCH_SIZE <= MAX_BATCH_WIRE_SIZE,
                  "MAX_BATCH_SIZE must fit in the BatchSize header field.");

    WorldBatchFramer();

    /**
     * Starts a new batch, discarding any unfinished one.
     */
    void begin();

    /**
     * Returns true if the current batch has no messages.
     */
    bool isEmpty() const;

    /**
     * Returns true if a framed message of the given size can fit in a batch.
     */
    static bool canEverFit(std::size_t framedMessageSize);

    /**
     * Returns true if a framed message of the given size would fit in the
     * current batch.
     */
    bool hasRoomFor(std::size_t framedMessageSize) const;

    /**
     * Appends an already-framed message to the current batch.
     * The message must fit (see hasRoomFor()).
     */
    void appendFramed(std::span<const Uint8> framedMessage);

    /**
     * Frames and appends the given message to the current batch.
     *
     * @return true if the message was appended, false if it didn't fit.
     */
    template<typename Message>
    bool appendMessage(const Message& message);

    /**
     * Fills in the current batch's header, compressing it if worthwhile.
     * Afterwards, a new batch is begun.
     *
     * @return The finished batch, ready to be sent.
     */
    BinaryBufferSharedPtr finish(Sint8 tickAdjustment,
                                 Uint8 adjustmentIteration);

private:
    /** Buffers larger than this aren't returned to the pool, so a few large
        batches don't permanently inflate our memory usage. */
    static constexpr std::size_t MAX_POOLED_BUFFER_CAPACITY{4096};

    /** The max number of buffers we'll keep in the pool. */
    static constexpr std::size_t MAX_POOLED_BUFFERS{4096};

    /** Holds buffers that are ready to be reused.
        Note: This is held through a shared_ptr so that buffers can safely
              return themselves to the pool, even if they outlive us. */
    struct BufferPool {
        std::vector<BinaryBufferPtr> freeBuffers{};
    };

    /**
     * Returns a buffer of the given size from the pool. When the returned
     * pointer is released, the buffer returns itself to the pool.
     */
    BinaryBufferSharedPtr acquireBuffer(std::size_t size);

    /** Holds the header and messages of the batch that's being built. */
    BinaryBuffer batchBuffer;

    /** If a batch is compressed, it's compressed into this buffer. */
    BinaryBuffer compressedBuffer;

    /** The index in batchBuffer where the next message will be written. */
    std::size_t currentIndex;

    std::shared_ptr<BufferPool> bufferPool;
};

template<typename Message>
bool WorldBatchFramer::appendMessage(const Message& message)
{
    std::size_t payloadSize{Serialize::measureSize(message)};
    if (!hasRoomFor(MESSAGE_HEADER_SIZE + payloadSize)) {
        return false;
    }

    // Serialize the message, leaving room for its header.
    Serialize::toBuffer(batchBuffer.data(), batchBuffer.size(), message,
                        (currentIndex + MESSAGE_HEADER_SIZE));

    // Fill in the message header.
    batchBuffer[currentIndex + MessageHeaderIndex::MessageType]
        = static_cast<Uint8>(Message::MESSAGE_TYPE);
    ByteTools::write16(
        static_cast<Uint16>(payloadSize),
        (batchBuffer.data() + currentIndex + MessageHeaderIndex::Size));

    currentIndex += MESSAGE_HEADER_SIZE + payloadSize;
    return true;
}

} // End namespace WorldServer
} // End namespace AM
