#include "WorldBatchFramer.h"
#include <algorithm>
#include <utility>

namespace AM
{
namespace WorldServer
{

WorldBatchFramer::WorldBatchFramer()
: batchBuffer(SharedConfig::MAX_BATCH_SIZE)
, compressedBuffer(ByteTools::compressBound(SharedConfig::MAX_BATCH_SIZE))
, currentIndex{SERVER_HEADER_SIZE}
, bufferPool{std::make_shared<BufferPool>()}
{
}

void WorldBatchFramer::begin()
{
    currentIndex = SERVER_HEADER_SIZE;
}

bool WorldBatchFramer::isEmpty() const
{
    return currentIndex == SERVER_HEADER_SIZE;
}

bool WorldBatchFramer::canEverFit(std::size_t framedMessageSize)
{
    return framedMessageSize
           <= (SharedConfig::MAX_BATCH_SIZE - SERVER_HEADER_SIZE);
}

bool WorldBatchFramer::hasRoomFor(std::size_t framedMessageSize) const
{
    return framedMessageSize <= (batchBuffer.size() - currentIndex);
}

void WorldBatchFramer::appendFramed(std::span<const Uint8> framedMessage)
{
    std::copy(framedMessage.begin(), framedMessage.end(),
              (batchBuffer.begin() + currentIndex));
    currentIndex += framedMessage.size();
}

BinaryBufferSharedPtr WorldBatchFramer::finish(Sint8 tickAdjustment,
                                               Uint8 adjustmentIteration)
{
    // If the batch is large enough, try compressing it.
    std::size_t batchSize{currentIndex - SERVER_HEADER_SIZE};
    const Uint8* batchToSend{batchBuffer.data() + SERVER_HEADER_SIZE};
    bool isCompressed{false};
    if (batchSize > SharedConfig::BATCH_COMPRESSION_THRESHOLD) {
        std::size_t compressedSize{
            ByteTools::compress(batchToSend, batchSize, compressedBuffer.data(),
                                compressedBuffer.size())};

        // Only use the compressed batch if it's actually smaller.
        if (compressedSize < batchSize) {
            batchToSend = compressedBuffer.data();
            batchSize = compressedSize;
            isCompressed = true;
        }
    }

    // Copy the batch into a buffer that the send can own.
    BinaryBufferSharedPtr batch{acquireBuffer(SERVER_HEADER_SIZE + batchSize)};
    std::copy(batchToSend, (batchToSend + batchSize),
              (batch->begin() + SERVER_HEADER_SIZE));

    // Fill in the header.
    (*batch)[ServerHeaderIndex::TickAdjustment]
        = static_cast<Uint8>(tickAdjustment);
    (*batch)[ServerHeaderIndex::AdjustmentIteration] = adjustmentIteration;

    // If the batch is compressed, set the high bit of its size.
    Uint16 batchSizeField{static_cast<Uint16>(batchSize)};
    if (isCompressed) {
        batchSizeField |= (1U << 15);
    }
    ByteTools::write16(batchSizeField,
                       &((*batch)[ServerHeaderIndex::BatchSize]));

    begin();
    return batch;
}

BinaryBufferSharedPtr WorldBatchFramer::acquireBuffer(std::size_t size)
{
    BinaryBufferPtr buffer{};
    if (!(bufferPool->freeBuffers.empty())) {
        buffer = std::move(bufferPool->freeBuffers.back());
        bufferPool->freeBuffers.pop_back();
    }
    else {
        buffer = std::make_unique<BinaryBuffer>();
    }
    buffer->resize(size);

    std::weak_ptr<BufferPool> weakPool{bufferPool};
    return BinaryBufferSharedPtr{
        buffer.release(), [weakPool](BinaryBuffer* releasedBuffer) {
            BinaryBufferPtr ownedBuffer{releasedBuffer};
            std::shared_ptr<BufferPool> pool{weakPool.lock()};
            if (pool && (ownedBuffer->capacity() <= MAX_POOLED_BUFFER_CAPACITY)
                && (pool->freeBuffers.size() < MAX_POOLED_BUFFERS)) {
                pool->freeBuffers.push_back(std::move(ownedBuffer));
            }
        }};
}

} // End namespace WorldServer
} // End namespace AM
