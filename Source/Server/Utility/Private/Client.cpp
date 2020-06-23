#include "Client.h"
#include "Peer.h"
#include "Debug.h"
#include <cmath>

namespace AM
{
namespace Server
{

Client::Client(std::unique_ptr<Peer> inPeer)
: peer(std::move(inPeer))
, hasRecordedDiff(false)
, adjustmentIteration(0)
{
    // Init the history.
    for (unsigned int i = 0; i < TICKDIFF_HISTORY_LENGTH; ++i) {
        tickDiffHistory.push(0);
    }
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

    // Receive the message.
    return peer->receiveMessage(false);
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
        DebugInfo("Dropped connection, diff out of bounds.");
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
        // No need to increment the iteration, we didn't do anything.
        return {0, adjustmentIteration};
    }

    // Acquire a lock so the tickDiffHistory doesn't change while we're processing it.
    std::unique_lock lock(tickDiffMutex);

    // Prep the adjustment that we'll return.
    Sint8 adjustment = 0;

    // If we potentially need to make an adjustment.
    Sint8 latestDiff = tickDiffHistory[0];
    Sint8 missedBy = TARGET_TICKDIFF - latestDiff;
    if (missedBy != 0) {
        float averageDiff = 0;
        for (unsigned int i = 0; i < TICKDIFF_HISTORY_LENGTH; ++i) {
            averageDiff += std::abs(tickDiffHistory[0]);
        }
        averageDiff /= TICKDIFF_HISTORY_LENGTH;

        // If it wasn't a lag spike.
        // (Best guess at detecting a lag spike, due for tweaking.)
        int lagBound = averageDiff * 2.0 + 3;
        if (missedBy < lagBound) {
            // Not a lag spike, use minor changes to walk the value in over time.
            if (missedBy > 0) {
                // Positive
                if (missedBy > 2) {
                    adjustment = 2;
                }
                else {
                    adjustment = 1;
                }
            }
            else if (missedBy < 0) {
                // Negative
                if (missedBy < -2) {
                    adjustment = -2;
                }
                else {
                    adjustment = -1;
                }
            }
        }
    }

    if (adjustment != 0) {
        adjustmentIteration++;
    }
    DebugInfo("latest: %d, missedBy: %d, adjustment: %d, iteration: %u", latestDiff,
        missedBy, adjustment, adjustmentIteration);
    return {adjustment, adjustmentIteration};
}

} // end namespace Server
} // end namespace AM

