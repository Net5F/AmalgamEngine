#include "Client.h"
#include "Peer.h"
#include "Debug.h"
#include "MessageSorter.h"
#include "Message_generated.h"
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
, latestSentSimTick(0)
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
    if ((latestSentSimTick == 0) && (messageCount == 0)) {
        // We haven't sent any data yet and have none to send, prevent a heartbeat
        // from being prematurely sent.
        return NetworkResult::Success;
    }

    /* Build the batch message. */
    // Copy any waiting messages into the buffer.
    unsigned int currentIndex = SERVER_HEADER_SIZE;
    for (unsigned int i = 0; i < messageCount; ++i) {
        /* Copy the message into the buffer. */
        BinaryBufferSharedPtr messageBuffer = sendQueue.front();
        std::copy(messageBuffer->begin(), messageBuffer->end(),
            &(batchBuffer[currentIndex]));

        currentIndex += messageBuffer->size();

        /* Track the latest tick we've sent. */
        // The message has a Uint16 messageSize in front of it.
        const fb::Message* message = fb::GetMessage(
            messageBuffer->data() + sizeof(Uint16));
        latestSentSimTick = message->tickTimestamp();
        DebugInfo("(%u) Update: updated latestSent to: %u", netID, latestSentSimTick);

        sendQueue.pop_front();
    }

    // Fill in the header.
    fillHeader(messageCount, currentTick);

    // Send the message.
    DebugInfo("Sending batch with messageCount: %u, confirmedTickCount: %u",
        batchBuffer[ServerHeaderIndex::MessageCount],
        batchBuffer[ServerHeaderIndex::ConfirmedTickCount]);
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

void Client::fillHeader(Uint8 messageCount, Uint32 currentTick)
{
    // Fill in the adjustment info.
    AdjustmentData tickAdjustment = getTickAdjustment();
    batchBuffer[ServerHeaderIndex::TickAdjustment] =
        static_cast<Uint8>(tickAdjustment.adjustment);
    batchBuffer[ServerHeaderIndex::AdjustmentIteration] = tickAdjustment.iteration;

    // Fill in the message count.
    batchBuffer[ServerHeaderIndex::MessageCount] = messageCount;

    // If we haven't sent data or are caught up, don't try to confirm any ticks.
    if ((latestSentSimTick == 0)
        || (latestSentSimTick == currentTick)) {
        batchBuffer[ServerHeaderIndex::ConfirmedTickCount] = 0;
        return;
    }
    else {
        // Fill in the number of ticks we've processed since the last update.
        // (the tick count increments at the end of a sim tick, so our latest sent
        //  data is from currentTick - 1).
        Uint8 confirmedTickCount = (currentTick - 1) - latestSentSimTick;
        batchBuffer[ServerHeaderIndex::ConfirmedTickCount] = confirmedTickCount;

        // Update our latestSent tracking to account for the confirmed ticks.
        latestSentSimTick += confirmedTickCount;

        // TEMP
        if (confirmedTickCount > 0) {
            DebugInfo("(%u) Confirm: updated latestSent to: %u", netID, latestSentSimTick);
        }
        // TEMP
    }
}


ReceiveResult Client::receiveMessage()
{
    if (peer == nullptr) {
        return {NetworkResult::Disconnected, nullptr};
    }

    // Receive the header.
    ReceiveResult result = peer->receiveBytes(CLIENT_HEADER_SIZE, false);

    // Check for timeouts.
    if (result.result == NetworkResult::Success) {
        // Process the adjustment iteration.
        const BinaryBuffer& header = *(result.message.get());
        Uint8 receivedAdjIteration = header[ClientHeaderIndex::AdjustmentIteration];
        Uint8 expectedNextIteration = (latestAdjIteration + 1);

        // If we received the next expected iteration, save it.
        if (receivedAdjIteration == expectedNextIteration) {
            latestAdjIteration = expectedNextIteration;
        }
        else if (receivedAdjIteration > expectedNextIteration) {
            DebugError("Skipped an adjustment iteration. Logic must be flawed.");
        }

        // Wait for the message.
        result = peer->receiveMessageWait();

        // Got a message, update the receiveTimer.
        receiveTimer.updateSavedTime();
    }
    else if (result.result == NetworkResult::NoWaitingData) {
        // If we timed out, drop the connection.
        if (double delta = receiveTimer.getDeltaSeconds(false) > TIMEOUT_S) {
            peer = nullptr;
            DebugInfo("Dropped connection, peer timed out. Time since last message: %.6f "
                "seconds. Timeout: %.6f", delta, TIMEOUT_S);
            return {NetworkResult::Disconnected, nullptr};
        }
    }

    return result;
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

