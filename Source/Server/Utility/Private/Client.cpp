#include "Client.h"
#include "Peer.h"
#include "Debug.h"
#include "MessageSorter.h"
#include <cmath>
#include <array>

namespace AM
{
namespace Server
{

Client::Client(NetworkID inNetID, std::unique_ptr<Peer> inPeer)
: netID(inNetID)
, peer(std::move(inPeer))
, batchBuffer{}
, hasRecordedDiff(false)
, latestAdjIteration(0)
{
    // Init the history.
    for (unsigned int i = 0; i < TICKDIFF_HISTORY_LENGTH; ++i) {
        tickDiffHistory.push(0);
    }

    // Init the timer to the current time.
    receiveTimer.updateSavedTime();
}

void Client::queueMessage(const BinaryBufferSharedPtr& message)
{
    sendQueue.push_back(message);
}

NetworkResult Client::sendWaitingMessages(Uint32 currentTick)
{
    if (peer == nullptr) {
        return NetworkResult::Disconnected;
    }

    Uint8 messageCount = getWaitingMessageCount();
    if (messageCount == 0) {
        // Nothing to send.
        return NetworkResult::Success;
    }

    /* Build the batch message. */
    // Copy any waiting messages into the buffer.
    unsigned int currentIndex = ServerHeaderIndex::MessageHeaderStart;
    for (unsigned int i = 0; i < messageCount; ++i) {
        /* Copy the message and message header into the buffer. */
        BinaryBufferSharedPtr messageBuffer = sendQueue.front();
        std::copy(messageBuffer->begin(), messageBuffer->end(),
            &(batchBuffer[currentIndex]));

        currentIndex += messageBuffer->size();

        sendQueue.pop_front();
    }

    // Fill in the header adjustment info.
    AdjustmentData tickAdjustment = getTickAdjustment();
    batchBuffer[ServerHeaderIndex::TickAdjustment] =
        static_cast<Uint8>(tickAdjustment.adjustment);
    batchBuffer[ServerHeaderIndex::AdjustmentIteration] = tickAdjustment.iteration;

    // Fill in the header message count.
    batchBuffer[ServerHeaderIndex::MessageCount] = messageCount;

    // Send the message.
    return peer->send(&(batchBuffer[0]), currentIndex);
}

Uint8 Client::getWaitingMessageCount() const
{
    unsigned int size = sendQueue.size();
    if (size > SDL_MAX_UINT8) {
        DebugError("Client's sendQueue contains too many messages to return as"
        "a Uint8.");
    }

    return size;
}

Message Client::receiveMessage()
{
    if (peer == nullptr) {
        return {NetworkResult::Disconnected, MessageType::NotSet, 0};
    }

    // Receive the header.
    Uint8 headerBuf[CLIENT_HEADER_SIZE];
    MessageResult result = peer->receiveBytes(headerBuf, CLIENT_HEADER_SIZE,
        false);

    // Receive the following message, or check for timeouts.
    if (result.networkResult == NetworkResult::Success) {
        // Process the adjustment iteration.
        Uint8 receivedAdjIteration = headerBuf[ClientHeaderIndex::AdjustmentIteration];
        Uint8 expectedNextIteration = (latestAdjIteration + 1);

        // If we received the next expected iteration, save it.
        if (receivedAdjIteration == expectedNextIteration) {
            latestAdjIteration = expectedNextIteration;
        }
        else if (receivedAdjIteration > expectedNextIteration) {
            DebugError("Skipped an adjustment iteration. Logic must be flawed.");
        }

        // Get the message.
        // Note: This is a blocking read, but the data should immediately be available
        //       since we send it all in 1 packet.
        BinaryBufferPtr messageBuffer = nullptr;
        result = peer->receiveMessageWait(messageBuffer);

        if (result.networkResult == NetworkResult::Success) {
            // Got a message, update the receiveTimer.
            receiveTimer.updateSavedTime();

            return {result.messageType, messageBuffer};
        }
    }
    else if (result.networkResult == NetworkResult::NoWaitingData) {
        // If we timed out, drop the connection.
        double delta = receiveTimer.getDeltaSeconds(false);
        if (delta > TIMEOUT_S) {
            peer = nullptr;
            DebugInfo("Dropped connection, peer timed out. Time since last message: %.6f "
                "seconds. Timeout: %.6f", delta, TIMEOUT_S);
            return {MessageType::NotSet, nullptr};
        }
    }

    return {MessageType::NotSet, nullptr};
}

bool Client::isConnected() {
    // Peer might've been force-disconnected by dropping the reference.
    // It also could have internally detected a client-initiated disconnect.
    return (peer == nullptr) ? false : peer->isConnected();
}

void Client::recordTickDiff(Sint64 tickDiff) {
    if ((tickDiff < LOWEST_VALID_TICKDIFF) || (tickDiff > HIGHEST_VALID_TICKDIFF)) {
        // Diff is outside our bounds. Drop the connection.
        peer = nullptr;
        DebugInfo("Dropped connection, diff out of bounds. Diff: %" PRId64 , tickDiff);
    }
    else {
        // Diff is fine, record it.
        // Acquire a lock so a getTickAdjustment doesn't start while we're pushing.
        std::unique_lock lock(tickDiffMutex);
        tickDiffHistory.push(tickDiff);

        // If this is the first diff we've recorded, init the buffer with it.
        if (!hasRecordedDiff) {
            for (unsigned int i = 0; i < (TICKDIFF_HISTORY_LENGTH - 1); ++i) {
                tickDiffHistory.push(tickDiff);
            }
            hasRecordedDiff = true;
        }
    }
}

Client::AdjustmentData Client::getTickAdjustment() {
    // If we haven't gotten any data, no adjustment should be made.
    if (!hasRecordedDiff) {
        return {0, 0};
    }

    // Copy the history so we can work on it without staying locked.
    std::unique_lock lock(tickDiffMutex);
    CircularBuffer<Sint8, TICKDIFF_HISTORY_LENGTH> tickDiffHistoryCopy = tickDiffHistory;
    lock.unlock();

    // Calc the average diff.
    float averageDiff = 0;
    for (unsigned int i = 0; i < TICKDIFF_HISTORY_LENGTH; ++i) {
        averageDiff += tickDiffHistory[i];
    }
    averageDiff /= TICKDIFF_HISTORY_LENGTH;

    // Run through all checks and calc any necessary adjustment.
    Sint8 adjustment = calcAdjustment(averageDiff, tickDiffHistoryCopy);

    return {adjustment, latestAdjIteration};
}

Sint8 Client::calcAdjustment(
float averageDiff, CircularBuffer<Sint8, TICKDIFF_HISTORY_LENGTH>& tickDiffHistoryCopy)
{
    if ((tickDiffHistoryCopy[0] >= TICKDIFF_ACCEPTABLE_BOUND_LOWER)
    && (tickDiffHistoryCopy[0] <= TICKDIFF_ACCEPTABLE_BOUND_UPPER)) {
        return 0;
    }

    // If the average isn't outside the target bounds, no adjustment is needed.
    int truncatedAverage = static_cast<int>(averageDiff);
    if ((truncatedAverage >= TICKDIFF_ACCEPTABLE_BOUND_LOWER)
    && (truncatedAverage <= TICKDIFF_ACCEPTABLE_BOUND_UPPER)) {
        return 0;
    }

    /* Check for lag spikes.
       Note: We check for lag spikes by seeing if we're ahead of the client (diff < target)
             and moving back towards the target.
             If we aren't, then we have to assume we had a long-term latency gain. */
    // If we're ahead of the client.
    if (tickDiffHistoryCopy[0] < TICKDIFF_TARGET) {
        // If we're moving back towards the target.
        if (tickDiffHistoryCopy[0] > tickDiffHistoryCopy[1]) {
            // It seems like a spike occurred, instead of a long-term latency gain.
            // No adjustment needed, it's going back to normal.
            return 0;
        }
    }

    // TEMP
    DebugInfo("Calc'd adjustment. adjustment: %d", TICKDIFF_TARGET - truncatedAverage);
//    DebugInfo("truncatedAverage: %d. Values:", static_cast<int>(averageDiff));
//    printf("[");
//    for (unsigned int i = 0; i < TICKDIFF_HISTORY_LENGTH; ++i) {
//        printf("%d, ", tickDiffHistoryCopy[i]);
//    }
//    printf("]\n");
    // TEMP

    // Make an adjustment back towards the target.
    return TICKDIFF_TARGET - truncatedAverage;
}

} // end namespace Server
} // end namespace AM

