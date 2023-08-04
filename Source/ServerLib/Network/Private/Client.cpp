#include "Client.h"
#include "Peer.h"
#include "Log.h"
#include "ByteTools.h"
#include "ExplicitConfirmation.h"
#include "Serialize.h"
#include "NetworkStats.h"
#include "AMAssert.h"
#include "Ignore.h"
#include <cmath>
#include <array>

namespace AM
{
namespace Server
{
BinaryBuffer Client::batchBuffer(SharedConfig::MAX_BATCH_SIZE);
// No default size since it's dynamically enlarged if too small.
BinaryBuffer Client::compressedBatchBuffer;

Client::Client(NetworkID inNetID, std::unique_ptr<Peer> inPeer)
: netID{inNetID}
, peer{std::move(inPeer)}
, receiveTimer{}
, latestSentSimTick{0}
, tickDiffHistory{Config::TICKDIFF_TARGET}
, numFreshDiffs{0}
, latestAdjIteration{0}
{
}

void Client::queueMessage(const BinaryBufferSharedPtr& message,
                          Uint32 messageTick)
{
    bool emplaceSucceeded{sendQueue.emplace(message, messageTick)};
    AM_ASSERT(emplaceSucceeded, "Queue emplace failed.");
    ignore(emplaceSucceeded);
}

NetworkResult Client::sendWaitingMessages(Uint32 currentTick)
{
    if (peer == nullptr) {
        return NetworkResult::Disconnected;
    }

    // If we have no messages to send, return early.
    std::size_t messageCount{getWaitingMessageCount()};
    if ((latestSentSimTick == 0) && (messageCount == 0)) {
        return NetworkResult::Success;
    }

    // Copy any waiting messages into the buffer.
    std::size_t currentIndex{ServerHeaderIndex::MessageHeaderStart};
    for (std::size_t i = 0; i < messageCount; ++i) {
        // Pop the message.
        QueuedMessage queuedMessage;
        bool dequeueSucceeded{sendQueue.try_dequeue(queuedMessage)};
        AM_ASSERT(dequeueSucceeded, "Expected element but dequeue failed.");
        ignore(dequeueSucceeded);

        // If the message would make the batch too large, error.
        std::size_t messageSize{queuedMessage.message->size()};
        AM_ASSERT(
            ((currentIndex + messageSize) <= SharedConfig::MAX_BATCH_SIZE),
            "Batch too large to fit into buffers. Increase MAX_BATCH_SIZE. "
            "Size: %u, Max: %u",
            (currentIndex + messageSize), SharedConfig::MAX_BATCH_SIZE);

        // Copy the message data into the batchBuffer.
        std::copy(queuedMessage.message->begin(), queuedMessage.message->end(),
                  &(batchBuffer[currentIndex]));

        // Increment the index.
        currentIndex += messageSize;

        // Track the latest tick we've sent.
        if (queuedMessage.tick != 0) {
            latestSentSimTick = queuedMessage.tick;
        }
    }

    // If we've started talking to this client and none of this batch's
    // messages confirm the latest tick, add an explicit confirmation message.
    if ((latestSentSimTick != 0) && (latestSentSimTick < (currentTick - 1))) {
        addExplicitConfirmation(currentIndex, currentTick);
    }

    // If the batch + header is too large, error.
    AM_ASSERT((currentIndex <= SharedConfig::MAX_BATCH_SIZE),
              "Batch too large to fit into buffers. Increase MAX_BATCH_SIZE. "
              "Size: %u, Max: %u",
              currentIndex, SharedConfig::MAX_BATCH_SIZE);

    // If we have a large enough payload, compress it.
    std::size_t batchSize{currentIndex - SERVER_HEADER_SIZE};
    Uint8* bufferToSend{&(batchBuffer[0])};
    bool isCompressed{false};
    if (batchSize > SharedConfig::BATCH_COMPRESSION_THRESHOLD) {
        batchSize = compressBatch(batchSize);

        isCompressed = true;

        // Use the compressed buffer.
        bufferToSend = &(compressedBatchBuffer[0]);
    }

    // Fill in the header.
    fillHeader(bufferToSend, static_cast<Uint16>(batchSize), isCompressed);

    // Record the number of sent bytes.
    std::size_t totalSize{SERVER_HEADER_SIZE + batchSize};
    NetworkStats::recordBytesSent(totalSize);

    // Send the header and batch.
    std::size_t sendIndex{0};
    NetworkResult result{NetworkResult::Success};
    while (sendIndex < totalSize) {
        // Calc how many bytes we have left to send.
        std::size_t bytesToSend{totalSize - sendIndex};

        // Only send up to MAX_WIRE_SIZE bytes per send() call.
        if (bytesToSend > Peer::MAX_WIRE_SIZE) {
            bytesToSend = Peer::MAX_WIRE_SIZE;
        }

        // Send the bytes.
        result = peer->send((bufferToSend + sendIndex), bytesToSend);
        if (result == NetworkResult::Disconnected) {
            break;
        }

        sendIndex += bytesToSend;
    }

    return result;
}

void Client::addExplicitConfirmation(std::size_t& currentIndex,
                                     Uint32 currentTick)
{
    /* Add the ExplicitConfirmation to the batch.
       Note: We add it by hand instead of using the normal functions because
             they're meant for queueing messages and this is post-queue. */

    // Write the message type.
    batchBuffer[currentIndex]
        = static_cast<Uint8>(EngineMessageType::ExplicitConfirmation);
    currentIndex++;

    // Write the message size.
    ByteTools::write16(1, &(batchBuffer[currentIndex]));
    currentIndex += 2;

    // Calc the number of ticks we've processed since the last update.
    // (the tick count increments at the end of a sim tick, so our latest
    //  sent data is from currentTick - 1).
    std::size_t confirmedTickCount{(currentTick - 1) - latestSentSimTick};
    AM_ASSERT(confirmedTickCount <= UINT8_MAX, "Count too large for Uint8.");

    // Write the explicit confirmation message.
    ExplicitConfirmation explicitConfirmation{
        static_cast<Uint8>(confirmedTickCount)};
    currentIndex += static_cast<std::size_t>(
        Serialize::toBuffer(batchBuffer.data(), batchBuffer.size(),
                            explicitConfirmation, currentIndex));

    // Update our latestSent tracking to account for the confirmed ticks.
    latestSentSimTick += static_cast<Uint32>(confirmedTickCount);
}

std::size_t Client::compressBatch(std::size_t batchSize)
{
    // If the destination buffer is too small, resize it.
    std::size_t compressBound{ByteTools::compressBound(batchSize)};
    if (compressedBatchBuffer.size() < compressBound) {
        compressedBatchBuffer.resize(compressBound);
    }

    // Compress the batch.
    std::size_t compressedBatchSize{
        static_cast<std::size_t>(ByteTools::compress(
            &(batchBuffer[ServerHeaderIndex::MessageHeaderStart]), batchSize,
            &(compressedBatchBuffer[ServerHeaderIndex::MessageHeaderStart]),
            compressedBatchBuffer.size()))};
    AM_ASSERT((compressedBatchSize <= MAX_BATCH_SIZE),
              "Batch too large, even after compression. Size: %u",
              compressedBatchSize);

    return compressedBatchSize;
}

void Client::fillHeader(Uint8* bufferToFill, Uint16 batchSize,
                        bool isCompressed)
{
    // Fill in the header adjustment info.
    AdjustmentData tickAdjustment{getTickAdjustment()};
    bufferToFill[ServerHeaderIndex::TickAdjustment]
        = static_cast<Uint8>(tickAdjustment.adjustment);
    bufferToFill[ServerHeaderIndex::AdjustmentIteration]
        = tickAdjustment.iteration;

    // If the payload is compressed, set the high bit.
    if (isCompressed) {
        batchSize |= (1U << 15);
    }

    // Fill in the compressed batch size.
    ByteTools::write16(batchSize,
                       &(bufferToFill[ServerHeaderIndex::BatchSize]));
}

std::size_t Client::getWaitingMessageCount() const
{
    return sendQueue.size_approx();
}

ReceiveResult Client::receiveMessage(Uint8* messageBuffer)
{
    if (peer == nullptr) {
        return {NetworkResult::Disconnected};
    }

    // Receive the header.
    Uint8 headerBuf[CLIENT_HEADER_SIZE];
    NetworkResult headerResult{
        peer->receiveBytes(headerBuf, CLIENT_HEADER_SIZE, false)};

    // Receive the following message, or check for timeouts.
    if (headerResult == NetworkResult::Success) {
        // Process the adjustment iteration.
        Uint8 receivedAdjIteration{
            headerBuf[ClientHeaderIndex::AdjustmentIteration]};
        Uint8 expectedNextIteration{static_cast<Uint8>(latestAdjIteration + 1)};

        // If we received the next expected iteration, save it.
        if (receivedAdjIteration == expectedNextIteration) {
            latestAdjIteration = expectedNextIteration;
            numFreshDiffs = 0;
        }
        else if (receivedAdjIteration > expectedNextIteration) {
            LOG_FATAL("Skipped an adjustment iteration. Logic must be flawed.");
        }

        // Get the message.
        // Note: This is a blocking read, but the data should immediately be
        //       available since we send it all in 1 packet.
        ReceiveResult receiveResult{peer->receiveMessageWait(messageBuffer)};
        if (receiveResult.networkResult == NetworkResult::Success) {
            // Got a message, update the receiveTimer.
            receiveTimer.reset();

            // Record the number of received bytes.
            NetworkStats::recordBytesReceived(CLIENT_HEADER_SIZE
                                              + MESSAGE_HEADER_SIZE
                                              + receiveResult.messageSize);

            return receiveResult;
        }
        else {
            LOG_FATAL("Data was not present when expected.");
        }
    }
    else if (headerResult == NetworkResult::NoWaitingData) {
        // If we timed out, drop the connection.
        double delta{receiveTimer.getTime()};
        if (delta > Config::CLIENT_TIMEOUT_S) {
            peer = nullptr;
            LOG_INFO("Dropped connection, peer timed out. Time since last "
                     "message: %.6f seconds. Timeout: %.6f, NetID: %u",
                     delta, Config::CLIENT_TIMEOUT_S, netID);
            return {NetworkResult::TimedOut};
        }
    }

    return {NetworkResult::NoWaitingData};
}

bool Client::isConnected()
{
    // Peer might've been force-disconnected by dropping the reference.
    // It also could have internally detected a client-initiated disconnect.
    return (peer == nullptr) ? false : peer->isConnected();
}

void Client::recordTickDiff(Sint64 tickDiff)
{
    // Acquire a lock so a getTickAdjustment() doesn't start while we're
    // pushing.
    std::unique_lock lock(tickDiffMutex);

    // Add the new data.
    if ((tickDiff >= Config::TICKDIFF_MAX_BOUND_LOWER)
        && (tickDiff <= Config::TICKDIFF_MAX_BOUND_UPPER)) {
        tickDiffHistory.push(static_cast<Sint8>(tickDiff));
    }
    else {
        // Tickdiff out of max range, drop the connection.
        peer = nullptr;
        LOG_INFO("Dropped connection, tickDiff out of range. tickDiff: %d, "
                 "NetID: %u",
                 tickDiff, netID);
        return;
    }

    // Note: This is safe, only this thread modifies numFreshDiffs.
    if (numFreshDiffs < Config::TICKDIFF_HISTORY_LENGTH) {
        numFreshDiffs++;
    }
}

Client::AdjustmentData Client::getTickAdjustment()
{
    // Copy the history so we can work on it without staying locked.
    std::unique_lock lock(tickDiffMutex);
    CircularBuffer<Sint8, Config::TICKDIFF_HISTORY_LENGTH> tickDiffHistoryCopy(
        tickDiffHistory);
    unsigned int numFreshDiffsCopy{numFreshDiffs};
    lock.unlock();

    // Run through all checks and calc any necessary adjustment.
    Sint8 adjustment{calcAdjustment(tickDiffHistoryCopy, numFreshDiffsCopy)};

    return {adjustment, latestAdjIteration};
}

NetworkID Client::getNetID()
{
    return netID;
}

Sint8 Client::calcAdjustment(
    CircularBuffer<Sint8, Config::TICKDIFF_HISTORY_LENGTH>& tickDiffHistoryCopy,
    unsigned int numFreshDiffsCopy)
{
    // If we don't have enough data, we won't make an adjustment.
    if (numFreshDiffsCopy < Config::MIN_FRESH_DIFFS) {
        return 0;
    }

    // If the latest data isn't outside the target bounds, no adjustment is
    // needed.
    if ((tickDiffHistoryCopy[0] >= Config::TICKDIFF_ACCEPTABLE_BOUND_LOWER)
        && (tickDiffHistoryCopy[0]
            <= Config::TICKDIFF_ACCEPTABLE_BOUND_UPPER)) {
        return 0;
    }

    // Calc the average diff using only fresh data.
    float averageDiff{0};
    for (unsigned int i = 0; i < numFreshDiffsCopy; ++i) {
        averageDiff += tickDiffHistoryCopy[i];
    }
    averageDiff /= numFreshDiffsCopy;

    // If the average isn't outside the target bounds, no adjustment is needed.
    int truncatedAverage{static_cast<int>(averageDiff)};
    if ((truncatedAverage >= Config::TICKDIFF_ACCEPTABLE_BOUND_LOWER)
        && (truncatedAverage <= Config::TICKDIFF_ACCEPTABLE_BOUND_UPPER)) {
        return 0;
    }

    /* Check for lag spikes.
       Note: We check for lag spikes by seeing if we're ahead of the client
             (diff < target) and moving back towards the target.
             If we aren't, then we have to assume we had a long-term latency
             gain. */
    // If we're ahead of the client.
    if (tickDiffHistoryCopy[0] < Config::TICKDIFF_TARGET) {
        // If we're moving back towards the target.
        if (tickDiffHistoryCopy[0] > tickDiffHistoryCopy[1]) {
            // It seems like a spike occurred, instead of a long-term latency
            // gain. No adjustment needed, it's going back to normal.
            return 0;
        }
    }

    printAdjustmentInfo(tickDiffHistoryCopy, numFreshDiffsCopy,
                        truncatedAverage);

    // Make an adjustment back towards the target.
    Sint64 adjustment{Config::TICKDIFF_TARGET - truncatedAverage};
    AM_ASSERT((adjustment >= SDL_MIN_SINT8), "Adjustment out of bounds.");
    AM_ASSERT((adjustment <= SDL_MAX_SINT8), "Adjustment out of bounds.");
    return static_cast<Sint8>(adjustment);
}

void Client::printAdjustmentInfo(
    const CircularBuffer<Sint8, Config::TICKDIFF_HISTORY_LENGTH>&
        tickDiffHistoryCopy,
    unsigned int numFreshDiffsCopy, int truncatedAverage)
{
    LOG_INFO("Calc'd adjustment. NetID: %u, adjustment: %d, iteration: %u",
             netID, (Config::TICKDIFF_TARGET - truncatedAverage),
             latestAdjIteration.load());
    LOG_INFO("truncatedAverage: %d, numFreshDiffs: %u. Values:",
             truncatedAverage, numFreshDiffsCopy);
    std::printf("[");
    for (unsigned int i = 0; i < Config::TICKDIFF_HISTORY_LENGTH; ++i) {
        std::printf("%d, ", tickDiffHistoryCopy[i]);
    }
    std::printf("]\n");
}

} // end namespace Server
} // end namespace AM
