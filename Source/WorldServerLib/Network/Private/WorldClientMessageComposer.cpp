#include "WorldClientMessageComposer.h"
#include "ByteTools.h"
#include "asio/error.hpp"

namespace AM
{
namespace WorldServer
{

WorldClientMessageComposer::WorldClientMessageComposer(
    std::size_t inMaxPayloadSize)
: state{ReadState::Header}
, maxPayloadSize{inMaxPayloadSize}
, headerBuffer{}
, payloadBuffer{}
, pendingAdjustmentIteration{0}
, pendingType{0}
, pendingPayloadSize{0}
{
    payloadBuffer.reserve(INITIAL_PAYLOAD_CAPACITY);
}

void WorldClientMessageComposer::setMaxPayloadSize(std::size_t inMaxPayloadSize)
{
    maxPayloadSize = inMaxPayloadSize;
}

asio::mutable_buffer WorldClientMessageComposer::nextReadBuffer()
{
    if (state == ReadState::Header) {
        return asio::buffer(headerBuffer);
    }
    return asio::buffer(payloadBuffer.data(), pendingPayloadSize);
}

WorldClientMessageComposer::ComposeResult
    WorldClientMessageComposer::onBytesRead()
{
    if (state == ReadState::Header) {
        return parseHeader();
    }
    return completeMessage({payloadBuffer.data(), pendingPayloadSize});
}

void WorldClientMessageComposer::reset()
{
    state = ReadState::Header;
    pendingAdjustmentIteration = 0;
    pendingType = 0;
    pendingPayloadSize = 0;
}

WorldClientMessageComposer::ComposeResult
    WorldClientMessageComposer::parseHeader()
{
    pendingAdjustmentIteration
        = headerBuffer[ClientHeaderIndex::AdjustmentIteration];
    pendingType
        = headerBuffer[CLIENT_HEADER_SIZE + MessageHeaderIndex::MessageType];
    pendingPayloadSize = ByteTools::read16(
        &(headerBuffer[CLIENT_HEADER_SIZE + MessageHeaderIndex::Size]));

    if (pendingPayloadSize > maxPayloadSize) {
        return makeError(
            asio::error::make_error_code(asio::error::message_size));
    }

    // If this is a 0-byte message, there's nothing more to read.
    if (pendingPayloadSize == 0) {
        return completeMessage({});
    }

    // If this message doesn't fit, grow straight to the max so that we only
    // grow once (and don't overshoot the max through geometric growth).
    if (pendingPayloadSize > payloadBuffer.capacity()) {
        payloadBuffer.reserve(maxPayloadSize);
    }
    payloadBuffer.resize(pendingPayloadSize);

    state = ReadState::Payload;
    return {};
}

WorldClientMessageComposer::ComposeResult
    WorldClientMessageComposer::completeMessage(std::span<Uint8> payload)
{
    ComposeResult result{};
    result.messageComplete = true;
    result.message = {.adjustmentIteration{pendingAdjustmentIteration},
                      .type{pendingType},
                      .payload{payload}};
    reset();
    return result;
}

WorldClientMessageComposer::ComposeResult
    WorldClientMessageComposer::makeError(const asio::error_code& error)
{
    reset();
    ComposeResult result{};
    result.error = error;
    return result;
}

} // End namespace WorldServer
} // End namespace AM
