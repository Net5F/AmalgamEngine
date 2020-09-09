#include "Client.h"
#include "Peer.h"
#include "Debug.h"
#include "MessageSorter.h"
#include "Message_generated.h"
#include <cmath>

namespace AM
{
namespace Server
{

Client::Client(std::unique_ptr<Peer> inPeer)
: peer(std::move(inPeer))
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

NetworkResult Client::sendHeader(const BinaryBufferSharedPtr& header)
{
    if (peer == nullptr) {
        return NetworkResult::Disconnected;
    }

    // If it's a heartbeat, update the latestSentSimTick.
    if ((header->at(ServerHeaderIndex::ConfirmedTickCount)
        & SERVER_HEARTBEAT_MASK) != 0) {
        Uint8 confirmedTickCount = header->at(ServerHeaderIndex::ConfirmedTickCount)
                                   ^ SERVER_HEARTBEAT_MASK;
        latestSentSimTick += confirmedTickCount;
        DebugInfo("(%p) Heartbeat: updated latestSent to: %u", this, latestSentSimTick);
    }

    return peer->send(header);
}

NetworkResult Client::sendWaitingMessages()
{
    if (peer == nullptr) {
        return NetworkResult::Disconnected;
    }

    while (!(sendQueue.empty())) {
        BinaryBufferSharedPtr messageBuffer = sendQueue.front();

        NetworkResult result = peer->send(messageBuffer);
        if (result != NetworkResult::Success) {
            // Some sort of failure, stop sending and return it.
            return result;
        }

        /* Track the latest tick we've sent. */
        // The message has a Uint16 messageSize in front of it.
        const fb::Message* message = fb::GetMessage(
            messageBuffer->data() + sizeof(Uint16));
        latestSentSimTick = message->tickTimestamp();
        DebugInfo("(%p) Batch: updated latestSent to: %u", this, latestSentSimTick);

        sendQueue.pop_front();
    }

    return NetworkResult::Success;
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

Uint8 Client::getWaitingMessageCount() const
{
    if (peer == nullptr) {
        return 0;
    }

    unsigned int size = sendQueue.size();
    if (size > SDL_MAX_UINT8) {
        DebugError("Client's sendQueue contains too many messages to return as"
        "a Uint8.");
    }

    return size;
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

    // TODO: Remove (debug)
    DebugInfo("Sent adjustment. adjustment: %d", TICKDIFF_TARGET - truncatedAverage);
    DebugInfo("truncatedAverage: %d. Values:", static_cast<int>(averageDiff));
    printf("[");
    for (unsigned int i = 0; i < TICKDIFF_HISTORY_LENGTH; ++i) {
        printf("%d, ", tickDiffHistoryCopy[i]);
    }
    printf("]\n");

    // Make an adjustment back towards the target.
    return TICKDIFF_TARGET - truncatedAverage;
}

Uint32 Client::getLatestSentSimTick()
{
    return latestSentSimTick;
}

} // end namespace Server
} // end namespace AM

