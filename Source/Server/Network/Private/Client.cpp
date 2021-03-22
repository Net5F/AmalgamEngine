#include "Client.h"
#include "Peer.h"
#include "Log.h"
#include "MessageSorter.h"
#include "NetworkStats.h"
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
, tickDiffHistory(Config::TICKDIFF_TARGET)
, numFreshDiffs(0)
, latestAdjIteration(0)
{
    // Init the timers to the current time.
    receiveTimer.updateSavedTime();
}

void Client::queueMessage(const BinaryBufferSharedPtr& message,
                          Uint32 messageTick)
{
    if (!sendQueue.emplace(message, messageTick)) {
        LOG_ERROR("Queue emplace failed.");
    }
}

NetworkResult Client::sendWaitingMessages(Uint32 currentTick)
{
    if (peer == nullptr) {
        return NetworkResult::Disconnected;
    }

    Uint8 messageCount = getWaitingMessageCount();
    if ((latestSentSimTick == 0) && (messageCount == 0)) {
        // Nothing to send.
        return NetworkResult::Success;
    }

    /* Build the batch message. */
    // Copy any waiting messages into the buffer.
    unsigned int currentIndex = ServerHeaderIndex::MessageHeaderStart;
    for (unsigned int i = 0; i < messageCount; ++i) {
        /* Copy the message and message header into the buffer. */
        std::pair<BinaryBufferSharedPtr, Uint32> messagePair;
        if (!sendQueue.try_dequeue(messagePair)) {
            LOG_ERROR("Expected element but dequeue failed.");
        }

        BinaryBufferSharedPtr& messageBuffer = messagePair.first;
        std::copy(messageBuffer->begin(), messageBuffer->end(),
                  &(batchBuffer[currentIndex]));

        currentIndex += messageBuffer->size();

        // Track the latest tick we've sent.
        Uint32 messageTick = messagePair.second;
        if (messageTick != 0) {
            latestSentSimTick = messageTick;
        }
    }

    // Fill in the header.
    fillHeader(messageCount, currentTick);

    // Record the number of sent bytes.
    NetworkStats::recordBytesSent(currentIndex);

    // Send the message.
    // Note: If there were no waiting messages, we still send the batch header
    // to confirm that no changes occurred.
    return peer->send(&(batchBuffer[0]), currentIndex);
}

void Client::fillHeader(Uint8 messageCount, Uint32 currentTick)
{
    // Fill in the header adjustment info.
    AdjustmentData tickAdjustment = getTickAdjustment();
    batchBuffer[ServerHeaderIndex::TickAdjustment]
        = static_cast<Uint8>(tickAdjustment.adjustment);
    batchBuffer[ServerHeaderIndex::AdjustmentIteration]
        = tickAdjustment.iteration;

    // Fill in the header message count.
    batchBuffer[ServerHeaderIndex::MessageCount] = messageCount;

    // If we haven't sent data or are caught up, don't try to confirm any ticks.
    if ((latestSentSimTick == 0) || (latestSentSimTick == currentTick)) {
        batchBuffer[ServerHeaderIndex::ConfirmedTickCount] = 0;
        return;
    }
    else {
        // Fill in the number of ticks we've processed since the last update.
        // (the tick count increments at the end of a sim tick, so our latest
        //  sent data is from currentTick - 1).
        Uint8 confirmedTickCount = (currentTick - 1) - latestSentSimTick;
        batchBuffer[ServerHeaderIndex::ConfirmedTickCount] = confirmedTickCount;

        // Update our latestSent tracking to account for the confirmed ticks.
        latestSentSimTick += confirmedTickCount;
    }
}

Uint8 Client::getWaitingMessageCount() const
{
    std::size_t size = sendQueue.size_approx();
    if (size > SDL_MAX_UINT8) {
        LOG_ERROR("Client's sendQueue contains too many messages to return as"
                  "a Uint8.");
    }

    return size;
}

Message Client::receiveMessage()
{
    if (peer == nullptr) {
        return {MessageType::NotSet, nullptr};
    }

    // Receive the header.
    Uint8 headerBuf[CLIENT_HEADER_SIZE];
    NetworkResult headerResult
        = peer->receiveBytes(headerBuf, CLIENT_HEADER_SIZE, false);

    // Receive the following message, or check for timeouts.
    if (headerResult == NetworkResult::Success) {
        // Process the adjustment iteration.
        Uint8 receivedAdjIteration
            = headerBuf[ClientHeaderIndex::AdjustmentIteration];
        Uint8 expectedNextIteration = (latestAdjIteration + 1);

        // If we received the next expected iteration, save it.
        if (receivedAdjIteration == expectedNextIteration) {
            latestAdjIteration = expectedNextIteration;
            numFreshDiffs = 0;
        }
        else if (receivedAdjIteration > expectedNextIteration) {
            LOG_ERROR("Skipped an adjustment iteration. Logic must be flawed.");
        }

        // Get the message.
        // Note: This is a blocking read, but the data should immediately be
        //       available since we send it all in 1 packet.
        BinaryBufferPtr messageBuffer = nullptr;
        MessageResult messageResult = peer->receiveMessageWait(messageBuffer);
        if (messageResult.networkResult == NetworkResult::Success) {
            // Got a message, update the receiveTimer.
            receiveTimer.updateSavedTime();

            // Record the number of received bytes.
            NetworkStats::recordBytesReceived(CLIENT_HEADER_SIZE
                                              + MESSAGE_HEADER_SIZE
                                              + messageBuffer->size());

            return {messageResult.messageType, std::move(messageBuffer)};
        }
        else {
            LOG_ERROR("Data was not present when expected.");
        }
    }
    else if (headerResult == NetworkResult::NoWaitingData) {
        // If we timed out, drop the connection.
        double delta = receiveTimer.getDeltaSeconds(false);
        if (delta > Config::CLIENT_TIMEOUT_S) {
            peer = nullptr;
            LOG_INFO("Dropped connection, peer timed out. Time since last "
                     "message: %.6f seconds. Timeout: %.6f, NetID: %u",
                     delta, Config::CLIENT_TIMEOUT_S, netID);
            return {MessageType::NotSet, nullptr};
        }
    }

    return {MessageType::NotSet, nullptr};
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
    if ((tickDiff > SDL_MIN_SINT8) && (tickDiff < SDL_MAX_SINT8)) {
        tickDiffHistory.push(tickDiff);
    }
    else {
        LOG_ERROR("tickDiff out of Sint8 range. diff: %" PRId64, tickDiff);
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
    unsigned int numFreshDiffsCopy = numFreshDiffs;
    lock.unlock();

    // Run through all checks and calc any necessary adjustment.
    Sint8 adjustment = calcAdjustment(tickDiffHistoryCopy, numFreshDiffsCopy);

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
        && (tickDiffHistoryCopy[0] <= Config::TICKDIFF_ACCEPTABLE_BOUND_UPPER)) {
        return 0;
    }

    // Calc the average diff using only fresh data.
    float averageDiff = 0;
    for (unsigned int i = 0; i < numFreshDiffsCopy; ++i) {
        averageDiff += tickDiffHistoryCopy[i];
    }
    averageDiff /= numFreshDiffsCopy;

    // If the average isn't outside the target bounds, no adjustment is needed.
    int truncatedAverage = static_cast<int>(averageDiff);
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
    return Config::TICKDIFF_TARGET - truncatedAverage;
}

void Client::printAdjustmentInfo(
    const CircularBuffer<Sint8, Config::TICKDIFF_HISTORY_LENGTH>& tickDiffHistoryCopy,
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
