#include "WorldBatchComposer.h"
#include "SharedConfig.h"
#include "NetworkDefs.h"
#include "ByteTools.h"
#include "asio/error.hpp"

namespace AM
{
namespace Client
{

WorldBatchComposer::WorldBatchComposer()
: state{ReadState::Header}
, headerBuffer(SERVER_HEADER_SIZE)
, batchBuffer(MAX_BATCH_WIRE_SIZE)
, decompressedBuffer(SharedConfig::MAX_BATCH_SIZE)
, pendingTickAdjustment{0}
, pendingAdjustmentIteration{0}
, pendingBatchSize{0}
, pendingBatchIsCompressed{false}
, messages{}
{
}

asio::mutable_buffer WorldBatchComposer::nextReadBuffer()
{
    if (state == ReadState::Header) {
        return asio::buffer(headerBuffer);
    }
    return asio::buffer(batchBuffer.data(), pendingBatchSize);
}

WorldBatchComposer::ComposeResult WorldBatchComposer::onBytesRead()
{
    if (state == ReadState::Header) {
        return parseHeader();
    }
    return parseBatch();
}

void WorldBatchComposer::reset()
{
    state = ReadState::Header;
    pendingTickAdjustment = 0;
    pendingAdjustmentIteration = 0;
    pendingBatchSize = 0;
    pendingBatchIsCompressed = false;
}

WorldBatchComposer::ComposeResult WorldBatchComposer::parseHeader()
{
    pendingTickAdjustment
        = static_cast<Sint8>(headerBuffer[ServerHeaderIndex::TickAdjustment]);
    pendingAdjustmentIteration
        = headerBuffer[ServerHeaderIndex::AdjustmentIteration];

    // Read the high bit of batchSize to tell whether the batch is compressed
    // or not. If the high bit is set, the batch is compressed.
    Uint16 batchSize{
        ByteTools::read16(&(headerBuffer[ServerHeaderIndex::BatchSize]))};
    pendingBatchIsCompressed = ((batchSize & (1U << 15)) != 0);

    // Reset the high bit of batchSize to get the real size.
    pendingBatchSize = (batchSize & ~(1U << 15));

    // Note: batchBuffer can hold any size that the header can describe, so we
    //       only need to check the uncompressed limit.
    if (!pendingBatchIsCompressed
        && (pendingBatchSize > SharedConfig::MAX_BATCH_SIZE)) {
        return makeError(
            asio::error::make_error_code(asio::error::message_size));
    }

    // If the batch is empty, there's nothing more to read.
    if (pendingBatchSize == 0) {
        if (pendingBatchIsCompressed) {
            return makeError(
                asio::error::make_error_code(asio::error::invalid_argument));
        }
        return completeBatch({});
    }

    state = ReadState::Batch;
    return {};
}

WorldBatchComposer::ComposeResult WorldBatchComposer::parseBatch()
{
    std::span<Uint8> batch{batchBuffer.data(), pendingBatchSize};

    // If the batch is compressed, decompress it.
    if (pendingBatchIsCompressed) {
        std::size_t decompressedSize{ByteTools::decompress(
            batchBuffer.data(), pendingBatchSize, decompressedBuffer.data(),
            decompressedBuffer.size())};
        batch = {decompressedBuffer.data(), decompressedSize};
    }

    return completeBatch(batch);
}

WorldBatchComposer::ComposeResult
    WorldBatchComposer::completeBatch(std::span<Uint8> batch)
{
    asio::error_code error{splitMessages(batch)};
    if (error) {
        return makeError(error);
    }

    ComposeResult result{};
    result.batchComplete = true;
    result.batch = {.tickAdjustment{pendingTickAdjustment},
                    .adjustmentIteration{pendingAdjustmentIteration},
                    .wireSize{SERVER_HEADER_SIZE + pendingBatchSize},
                    .messages{messages}};
    reset();
    return result;
}

asio::error_code WorldBatchComposer::splitMessages(std::span<Uint8> batch)
{
    messages.clear();

    std::size_t index{0};
    while (index < batch.size()) {
        if ((batch.size() - index) < MESSAGE_HEADER_SIZE) {
            return asio::error::make_error_code(asio::error::invalid_argument);
        }

        Uint8 messageType{batch[index + MessageHeaderIndex::MessageType]};
        std::size_t messageSize{
            ByteTools::read16(&(batch[index + MessageHeaderIndex::Size]))};
        std::size_t payloadStart{index + MESSAGE_HEADER_SIZE};
        if (messageSize > (batch.size() - payloadStart)) {
            return asio::error::make_error_code(asio::error::invalid_argument);
        }

        messages.push_back(
            {messageType, batch.subspan(payloadStart, messageSize)});
        index = payloadStart + messageSize;
    }

    return {};
}

WorldBatchComposer::ComposeResult
    WorldBatchComposer::makeError(const asio::error_code& error)
{
    reset();
    ComposeResult result{};
    result.error = error;
    return result;
}

} // namespace Client
} // namespace AM
