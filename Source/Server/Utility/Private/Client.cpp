#include "Client.h"
#include "Peer.h"
#include "Debug.h"
#include "MessageSorter.h"
#include <cmath>

namespace AM
{
namespace Server
{

Client::Client(std::unique_ptr<Peer> inPeer)
: peer(std::move(inPeer))
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

    return peer->send(header);
}

NetworkResult Client::sendWaitingMessages()
{
    if (peer == nullptr) {
        return NetworkResult::Disconnected;
    }

    while (!(sendQueue.empty())) {
        BinaryBufferSharedPtr message = sendQueue.front();

        NetworkResult result = peer->send(message);
        if (result != NetworkResult::Success) {
            // Some sort of failure, stop sending and return it.
            return result;
        }

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
        if (float delta = receiveTimer.getDeltaSeconds(false) > TIMEOUT_S) {
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
        DebugInfo("Dropped connection, diff out of bounds. Diff: %lld", tickDiff);
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

    // Prep the adjustment that we'll return.
    Sint8 adjustment = 0;

    /* Get the average diff. */
    std::unique_lock lock(tickDiffMutex);

    float averageDiff = 0;
    for (unsigned int i = 0; i < TICKDIFF_HISTORY_LENGTH; ++i) {
        averageDiff += tickDiffHistory[i];
    }
    averageDiff /= TICKDIFF_HISTORY_LENGTH;

    // We're done accessing the tickDiffHistory.
    lock.unlock();

    /* If we're outside the target bounds, calculate an adjustment. */
    int truncatedAverage = static_cast<int>(averageDiff);
    if ((truncatedAverage < TICKDIFF_ACCEPTABLE_BOUND_LOWER)
    || (truncatedAverage > TICKDIFF_ACCEPTABLE_BOUND_UPPER)) {
        adjustment = TICKDIFF_TARGET - truncatedAverage;
    }

    // TODO: Remove this. Just need it to avoid copy in DebugInfo call.
    Uint8 tempLatestAdjIter = latestAdjIteration;
    if (adjustment != 0) {
        DebugInfo("Sent adjustment. adjustment: %d, iteration: %u", adjustment,
            tempLatestAdjIter);
        DebugInfo("truncatedAverage: %d. Values:", truncatedAverage);
        printf("[");
        for (unsigned int i = 0; i < TICKDIFF_HISTORY_LENGTH; ++i) {
            printf("%d, ", tickDiffHistory[i]);
        }
        printf("]\n");
    }
    return {adjustment, tempLatestAdjIter};
}

} // end namespace Server
} // end namespace AM

