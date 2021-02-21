#include "Network.h"
#include "Peer.h"
#include "MessageTools.h"
#include "EntityUpdate.h"
#include "ConnectionResponse.h"
#include "Heartbeat.h"
#include "NetworkStats.h"
#include <SDL_net.h>

namespace AM
{
namespace Client
{
Network::Network()
: server(nullptr)
, messageHandler(*this)
, playerEntity(entt::null)
, tickAdjustment(0)
, adjustmentIteration(0)
, isApplyingTickAdjustment(false)
, messagesSentSinceTick(0)
, currentTickPtr(nullptr)
, receiveThreadObj()
, exitRequested(false)
, headerRecBuffer(SERVER_HEADER_SIZE)
, messageRecBuffer(Peer::MAX_MESSAGE_SIZE)
, netstatsLoggingEnabled(true)
, ticksSinceNetstatsLog(0)
{
    if (!RUN_OFFLINE) {
        SDLNet_Init();
    }

    // Init the timer to the current time.
    receiveTimer.updateSavedTime();
}

Network::~Network()
{
    exitRequested = true;

    if (!RUN_OFFLINE) {
        receiveThreadObj.join();
        SDLNet_Quit();
    }
}

bool Network::connect()
{
    // Try to connect.
    server = Peer::initiate(SERVER_IP, SERVER_PORT);

    // Spin up the receive thread.
    if (server != nullptr) {
        receiveThreadObj = std::thread(&Network::pollForMessages, this);
    }

    return (server != nullptr);
}

void Network::tick()
{
    if (!RUN_OFFLINE) {
        // Send a heartbeat if we need to.
        sendHeartbeatIfNecessary();

        // If it's time to log our network statistics, do so.
        if (netstatsLoggingEnabled) {
            ticksSinceNetstatsLog++;
            if (ticksSinceNetstatsLog == TICKS_TILL_STATS_DUMP) {
                logNetworkStatistics();
                ticksSinceNetstatsLog = 0;
            }
        }
    }
}

void Network::send(const BinaryBufferSharedPtr& message)
{
    if ((server == nullptr) || !(server->isConnected())) {
        LOG_ERROR("Tried to send while server is disconnected.");
    }

    // Fill the message with the header (constructMessage() leaves
    // CLIENT_HEADER_SIZE bytes empty at the front for us to fill.)
    message->at(ClientHeaderIndex::AdjustmentIteration) = adjustmentIteration;

    // Send the message.
    NetworkResult result = server->send(message);
    if (result == NetworkResult::Success) {
        messagesSentSinceTick++;

        // Record the number of sent bytes.
        NetworkStats::recordBytesSent(message->size());
    }
    else {
        LOG_ERROR("Message send failed.");
    }
}

std::unique_ptr<ConnectionResponse>
    Network::receiveConnectionResponse(Uint64 timeoutMs)
{
    std::unique_ptr<ConnectionResponse> message = nullptr;
    if (timeoutMs == 0) {
        messageHandler.connectionResponseQueue.try_dequeue(message);
    }
    else {
        messageHandler.connectionResponseQueue.wait_dequeue_timed(
            message, timeoutMs * 1000);
    }

    return message;
}

std::shared_ptr<const EntityUpdate>
    Network::receivePlayerUpdate(Uint64 timeoutMs)
{
    std::shared_ptr<const EntityUpdate> message = nullptr;
    if (timeoutMs == 0) {
        messageHandler.playerUpdateQueue.try_dequeue(message);
    }
    else {
        messageHandler.playerUpdateQueue.wait_dequeue_timed(message,
                                                            timeoutMs * 1000);
    }

    return message;
}

NpcReceiveResult Network::receiveNpcUpdate(Uint64 timeoutMs)
{
    NpcUpdateMessage message;
    bool messageWasReceived = false;
    if (timeoutMs == 0) {
        messageWasReceived = messageHandler.npcUpdateQueue.try_dequeue(message);
    }
    else {
        messageWasReceived = messageHandler.npcUpdateQueue.wait_dequeue_timed(
            message, timeoutMs * 1000);
    }

    if (!messageWasReceived) {
        return {NetworkResult::NoWaitingData, {}};
    }
    else {
        return {NetworkResult::Success, message};
    }
}

int Network::pollForMessages()
{
    while (!exitRequested) {
        // Wait for a server header.
        NetworkResult headerResult = server->receiveBytesWait(
            headerRecBuffer.data(), SERVER_HEADER_SIZE);

        if (headerResult == NetworkResult::Success) {
            processBatch();
        }
        else if (headerResult == NetworkResult::Disconnected) {
            LOG_ERROR("Found server to be disconnected while trying to "
                      "receive header.");
        }
    }

    return 0;
}

int Network::transferTickAdjustment()
{
    if (isApplyingTickAdjustment) {
        int currentAdjustment = tickAdjustment;
        if (currentAdjustment < 0) {
            // The sim can only freeze for 1 tick at a time, transfer 1 from
            // tickAdjustment.
            tickAdjustment += 1;
            return -1;
        }
        else if (currentAdjustment > 0) {
            // The sim can process multiple iterations to catch up, transfer
            // all of tickAdjustment.
            tickAdjustment -= currentAdjustment;
            return currentAdjustment;
        }
        else {
            // We finished applying the adjustment, increment the iteration.
            adjustmentIteration++;
            isApplyingTickAdjustment = false;
            return 0;
        }
    }
    else {
        return 0;
    }
}

void Network::registerCurrentTickPtr(
    const std::atomic<Uint32>* inCurrentTickPtr)
{
    currentTickPtr = inCurrentTickPtr;
}

void Network::setPlayerEntity(entt::entity inPlayerEntity)
{
    playerEntity = inPlayerEntity;
}

entt::entity Network::getPlayerEntity()
{
    return playerEntity;
}

void Network::setNetstatsLoggingEnabled(bool inNetstatsLoggingEnabled)
{
    netstatsLoggingEnabled = inNetstatsLoggingEnabled;
}

void Network::sendHeartbeatIfNecessary()
{
    if (messagesSentSinceTick == 0) {
        // Prepare a heartbeat.
        Heartbeat heartbeat{};
        heartbeat.tickNum = *currentTickPtr;

        // Serialize the heartbeat message.
        BinaryBufferSharedPtr messageBuffer
            = std::make_shared<BinaryBuffer>(Peer::MAX_MESSAGE_SIZE);
        unsigned int startIndex = CLIENT_HEADER_SIZE + MESSAGE_HEADER_SIZE;
        std::size_t messageSize
            = MessageTools::serialize(*messageBuffer, heartbeat, startIndex);

        // Fill the buffer with the appropriate message header.
        MessageTools::fillMessageHeader(MessageType::Heartbeat, messageSize,
                                        messageBuffer, CLIENT_HEADER_SIZE);

        // Send the message.
        send(messageBuffer);
    }

    messagesSentSinceTick = 0;
}

void Network::processBatch()
{
    // Start tracking the number of received bytes.
    unsigned int bytesReceived = SERVER_HEADER_SIZE;

    // Check if we need to adjust the tick offset.
    adjustIfNeeded(headerRecBuffer[ServerHeaderIndex::TickAdjustment],
                   headerRecBuffer[ServerHeaderIndex::AdjustmentIteration]);

    /* Process messages, if we received any. */
    Uint8 messageCount = headerRecBuffer[ServerHeaderIndex::MessageCount];
    for (unsigned int i = 0; i < messageCount; ++i) {
        MessageResult messageResult
            = server->receiveMessageWait(messageRecBuffer.data());

        // If we received a message, push it into the appropriate queue.
        if (messageResult.networkResult == NetworkResult::Success) {
            // Got a message, process it and update the receiveTimer.
            processReceivedMessage(messageResult.messageType,
                                   messageResult.messageSize);
            receiveTimer.updateSavedTime();

            // Track the number of bytes we've received.
            bytesReceived += MESSAGE_HEADER_SIZE + messageResult.messageSize;
        }
        else if ((messageResult.networkResult == NetworkResult::NoWaitingData)
                 && (receiveTimer.getDeltaSeconds(false) > SERVER_TIMEOUT_S)) {
            // Too long since we received a message, timed out.
            LOG_ERROR("Server connection timed out.");
        }
        else if (messageResult.networkResult == NetworkResult::Disconnected) {
            LOG_ERROR("Found server to be disconnected while trying to "
                      "receive message.");
        }
    }

    /* Process any confirmed ticks. */
    Uint8 confirmedTickCount
        = headerRecBuffer[ServerHeaderIndex::ConfirmedTickCount];
    for (unsigned int i = 0; i < confirmedTickCount; ++i) {
        if (!(messageHandler.npcUpdateQueue.enqueue(
                {NpcUpdateType::ExplicitConfirmation}))) {
            LOG_ERROR("Ran out of room in queue and memory allocation failed.");
        }
    }

    // Record the number of received bytes.
    NetworkStats::recordBytesReceived(bytesReceived);
}

void Network::processReceivedMessage(MessageType messageType,
                                     Uint16 messageSize)
{
    /* Route the message to the appropriate handler. */
    switch (messageType) {
        case MessageType::ConnectionResponse:
            messageHandler.handleConnectionResponse(messageRecBuffer,
                                                    messageSize);
            break;
        case MessageType::EntityUpdate:
            messageHandler.handleEntityUpdate(messageRecBuffer, messageSize);
            break;
        default:
            LOG_ERROR("Received unexpected message type: %u", messageType);
    }
}

void Network::adjustIfNeeded(Sint8 receivedTickAdj, Uint8 receivedAdjIteration)
{
    if (receivedTickAdj != 0) {
        Uint8 currentAdjIteration = adjustmentIteration;

        // If it's the current iteration and we aren't already applying it.
        if ((receivedAdjIteration == currentAdjIteration)
            && !isApplyingTickAdjustment) {
            // Set the adjustment to be applied.
            tickAdjustment += receivedTickAdj;
            isApplyingTickAdjustment = true;
            LOG_INFO("Received tick adjustment: %d, iteration: %u",
                     receivedTickAdj, receivedAdjIteration);
        }
        else if (receivedAdjIteration > currentAdjIteration) {
            if (isApplyingTickAdjustment) {
                LOG_ERROR("Received future adjustment iteration while applying "
                          "the last. current: %u, received: %u",
                          currentAdjIteration, receivedAdjIteration);
            }
            else {
                LOG_ERROR("Out of sequence adjustment iteration. current: %u, "
                          "received: %u",
                          currentAdjIteration, receivedAdjIteration);
            }
        }
    }
}

void Network::logNetworkStatistics()
{
    // Dump the stats from the tracker.
    NetStatsDump netStats = NetworkStats::dumpStats();

    // Log the stats.
    float bytesSentPerSecond = netStats.bytesSent / SECONDS_TILL_STATS_DUMP;
    float bytesReceivedPerSecond
        = netStats.bytesReceived / SECONDS_TILL_STATS_DUMP;
    LOG_INFO("Bytes sent per second: %.0f, Bytes received per second: %.0f",
             bytesSentPerSecond, bytesReceivedPerSecond);
}

} // namespace Client
} // namespace AM
