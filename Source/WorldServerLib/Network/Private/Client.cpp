#include "Client.h"
#include "WorldBatchFramer.h"
#include "ExplicitConfirmation.h"
#include "Log.h"
#include "AMAssert.h"
#include <cstdio>
#include <utility>

namespace AM
{
namespace WorldServer
{
Client::Client(std::shared_ptr<WorldClientConnection> inConnection)
: connection{std::move(inConnection)}
, handle{}
, disconnecting{false}
, authState{AuthState::AwaitingRequest}
, accountID{0}
, queuedMessages{}
, queuedDisconnect{}
, latestSentSimTick{0}
, tickDiffHistory{Config::TICKDIFF_TARGET}
, numFreshDiffs{0}
, latestAdjIteration{0}
{
}

void Client::close()
{
    connection->close();
}

void Client::setHandle(ConnectionHandle inHandle)
{
    handle = inHandle;
}

ConnectionHandle Client::getHandle() const
{
    return handle;
}

WorldClientConnection& Client::getConnection()
{
    return *connection;
}

Client::AuthState Client::getAuthState() const
{
    return authState;
}

void Client::setAuthState(AuthState inAuthState)
{
    authState = inAuthState;
}

Sint64 Client::getAccountID() const
{
    return accountID;
}

void Client::setAccountID(Sint64 inAccountID)
{
    accountID = inAccountID;
}

bool Client::isDisconnecting() const
{
    return disconnecting;
}

void Client::setDisconnecting()
{
    disconnecting = true;
}

bool Client::processAdjustmentIteration(Uint8 receivedIteration)
{
    // If we received the next expected iteration, save it.
    Uint8 expectedNextIteration{static_cast<Uint8>(latestAdjIteration + 1)};
    if (receivedIteration == expectedNextIteration) {
        latestAdjIteration = expectedNextIteration;
        numFreshDiffs = 0;
        return true;
    }

    // Clients only move to the next iteration after we send them an
    // adjustment, so anything else is invalid.
    return receivedIteration == latestAdjIteration;
}

bool Client::recordTickDiff(Sint64 tickDiff)
{
    // If the diff is out of our valid range, the client should be dropped.
    if ((tickDiff < Config::TICKDIFF_MAX_BOUND_LOWER)
        || (tickDiff > Config::TICKDIFF_MAX_BOUND_UPPER)) {
        return false;
    }

    tickDiffHistory.push(static_cast<Sint8>(tickDiff));
    if (numFreshDiffs < Config::TICKDIFF_HISTORY_LENGTH) {
        numFreshDiffs++;
    }

    return true;
}

void Client::queueMessage(BinaryBufferSharedPtr message, Uint32 messageTick)
{
    // If we're going to disconnect, drop any further messages.
    if (queuedDisconnect) {
        return;
    }

    queuedMessages.emplace_back(std::move(message), messageTick);
}

void Client::queueDisconnect(DisconnectNotice::Reason reason)
{
    if (!queuedDisconnect) {
        queuedDisconnect = reason;
    }
}

std::optional<DisconnectNotice::Reason> Client::getQueuedDisconnect() const
{
    return queuedDisconnect;
}

std::size_t Client::sendQueuedMessages(WorldBatchFramer& framer,
                                       Uint32 currentTick)
{
    // If we haven't started talking to this client and have nothing to send,
    // there's nothing to do.
    if ((latestSentSimTick == 0) && queuedMessages.empty()
        && !queuedDisconnect) {
        return 0;
    }

    AdjustmentData adjustmentData{getTickAdjustment()};
    std::size_t bytesSent{0};
    bool batchWasSent{false};
    framer.begin();

    // Add the queued messages to the batch, sending it whenever it fills up.
    for (const QueuedMessage& queuedMessage : queuedMessages) {
        std::size_t messageSize{queuedMessage.message->size()};
        if (!WorldBatchFramer::canEverFit(messageSize)) {
            LOG_ERROR("Dropped a message that's too large to fit in a batch. "
                      "Size: %u, ClientID: %u",
                      messageSize, getHandle().id());
            continue;
        }
        if (!framer.hasRoomFor(messageSize)) {
            bytesSent += sendBatch(framer, adjustmentData);
            batchWasSent = true;
        }

        framer.appendFramed(*(queuedMessage.message));

        // Track the latest tick we've sent.
        if (queuedMessage.tick != 0) {
            latestSentSimTick = queuedMessage.tick;
        }
    }
    queuedMessages.clear();

    // If we've started talking to this client and none of this batch's
    // messages confirm the latest tick, add an explicit confirmation.
    // Note: The tick count increments at the end of a sim tick, so our latest
    //       sent data is from currentTick - 1.
    if ((latestSentSimTick != 0) && (latestSentSimTick < (currentTick - 1))) {
        std::size_t confirmedTickCount{(currentTick - 1) - latestSentSimTick};
        AM_ASSERT(confirmedTickCount <= UINT8_MAX,
                  "Count too large for Uint8.");

        bytesSent += appendMessage(
            framer,
            ExplicitConfirmation{static_cast<Uint8>(confirmedTickCount)},
            adjustmentData, batchWasSent);
        latestSentSimTick += static_cast<Uint32>(confirmedTickCount);
    }

    // If we're going to disconnect, tell the client why.
    if (queuedDisconnect) {
        bytesSent += appendMessage(framer, DisconnectNotice{*queuedDisconnect},
                                   adjustmentData, batchWasSent);
    }

    // Send the final batch.
    // Note: If nothing else was sent, we send it even if it's empty, since
    //       the header carries our tick adjustment.
    if (!framer.isEmpty() || !batchWasSent) {
        bytesSent += sendBatch(framer, adjustmentData);
    }

    return bytesSent;
}

std::size_t Client::sendBatch(WorldBatchFramer& framer,
                              const AdjustmentData& adjustmentData)
{
    BinaryBufferSharedPtr batch{
        framer.finish(adjustmentData.adjustment, adjustmentData.iteration)};
    std::size_t batchSize{batch->size()};

    // Note: If this fails, our connection will report the failure through
    //       its disconnect callback.
    if (!(connection->sendFramed(std::move(batch)))) {
        return 0;
    }

    return batchSize;
}

Client::AdjustmentData Client::getTickAdjustment()
{
    return {calcAdjustment(), latestAdjIteration};
}

Sint8 Client::calcAdjustment()
{
    // If we don't have enough data, we won't make an adjustment.
    if (numFreshDiffs < Config::MIN_FRESH_DIFFS) {
        return 0;
    }

    // If the latest data isn't outside the target bounds, no adjustment is
    // needed.
    if ((tickDiffHistory[0] >= Config::TICKDIFF_ACCEPTABLE_BOUND_LOWER)
        && (tickDiffHistory[0] <= Config::TICKDIFF_ACCEPTABLE_BOUND_UPPER)) {
        return 0;
    }

    // Calc the average diff using only fresh data.
    float averageDiff{0};
    for (unsigned int i = 0; i < numFreshDiffs; ++i) {
        averageDiff += tickDiffHistory[i];
    }
    averageDiff /= numFreshDiffs;

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
    if (tickDiffHistory[0] < Config::TICKDIFF_TARGET) {
        // If we're moving back towards the target.
        if (tickDiffHistory[0] > tickDiffHistory[1]) {
            // It seems like a spike occurred, instead of a long-term latency
            // gain. No adjustment needed, it's going back to normal.
            return 0;
        }
    }

    printAdjustmentInfo(truncatedAverage);

    // Make an adjustment back towards the target.
    Sint64 adjustment{Config::TICKDIFF_TARGET - truncatedAverage};
    AM_ASSERT((adjustment >= SDL_MIN_SINT8), "Adjustment out of bounds.");
    AM_ASSERT((adjustment <= SDL_MAX_SINT8), "Adjustment out of bounds.");
    return static_cast<Sint8>(adjustment);
}

void Client::printAdjustmentInfo([[maybe_unused]] int truncatedAverage)
{
    LOG_DEBUG("Calc'd adjustment. ClientID: %u, adjustment: %d, iteration: %u",
              getHandle().id(), (Config::TICKDIFF_TARGET - truncatedAverage),
              latestAdjIteration);
    LOG_DEBUG("truncatedAverage: %d, numFreshDiffs: %u. Values:",
              truncatedAverage, numFreshDiffs);
#ifdef DEBUG
    std::printf("[");
    for (unsigned int i{0}; i < Config::TICKDIFF_HISTORY_LENGTH; ++i) {
        std::printf("%d, ", tickDiffHistory[i]);
    }
    std::printf("]\n");
#endif
}

template<typename Message>
std::size_t Client::appendMessage(WorldBatchFramer& framer,
                                  const Message& message,
                                  const AdjustmentData& adjustmentData,
                                  bool& batchWasSent)
{
    if (framer.appendMessage(message)) {
        return 0;
    }

    // The batch is full. Send it and start a new one.
    std::size_t bytesSent{sendBatch(framer, adjustmentData)};
    batchWasSent = true;
    framer.appendMessage(message);
    return bytesSent;
}

} // end namespace WorldServer
} // end namespace AM
