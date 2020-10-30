#include "Network.h"
#include "Peer.h"
#include "ConnectionResponse.h"
#include "MessageTools.h"
#include "EntityUpdate.h"
#include "Heartbeat.h"
#include "NetworkStats.h"
#include <SDL2/SDL_net.h>

namespace AM
{
namespace Client
{
Network::Network()
: accumulatedTime(0.0)
, server(nullptr)
, playerID(INVALID_ENTITY_ID)
, tickAdjustment(0)
, adjustmentIteration(0)
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
    accumulatedTime += tickTimer.getDeltaSeconds(true);

    if (accumulatedTime >= NETWORK_TICK_TIMESTEP_S) {
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

        accumulatedTime -= NETWORK_TICK_TIMESTEP_S;
        if (accumulatedTime >= NETWORK_TICK_TIMESTEP_S) {
            // If we've accumulated enough time to send more, something
            // happened to delay us.
            // We still only want to send what's in the queue, but it's worth
            // giving debug output that we detected this.
            LOG_INFO("Detected a delayed network tick. accumulatedTime: %f. "
                     "Setting to 0.",
                     accumulatedTime);
            accumulatedTime = 0;
        }
    }
}

void Network::send(const BinaryBufferSharedPtr& message)
{
    if (!(server->isConnected())) {
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
        connectionResponseQueue.try_dequeue(message);
    }
    else {
        connectionResponseQueue.wait_dequeue_timed(message, timeoutMs * 1000);
    }

    return message;
}

std::shared_ptr<const EntityUpdate>
    Network::receivePlayerUpdate(Uint64 timeoutMs)
{
    std::shared_ptr<const EntityUpdate> message = nullptr;
    if (timeoutMs == 0) {
        playerUpdateQueue.try_dequeue(message);
    }
    else {
        playerUpdateQueue.wait_dequeue_timed(message, timeoutMs * 1000);
    }

    return message;
}

NpcReceiveResult Network::receiveNpcUpdate(Uint64 timeoutMs)
{
    NpcUpdateMessage message;
    bool messageWasReceived = false;
    if (timeoutMs == 0) {
        messageWasReceived = npcUpdateQueue.try_dequeue(message);
    }
    else {
        messageWasReceived
            = npcUpdateQueue.wait_dequeue_timed(message, timeoutMs * 1000);
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
    int currentAdjustment = tickAdjustment;
    if (currentAdjustment < 0) {
        // The sim can only freeze for 1 tick at a time.
        tickAdjustment += 1;
        return currentAdjustment;
    }
    else if (currentAdjustment == 0) {
        return 0;
    }
    else {
        // The sim can process multiple iterations to catch up.
        tickAdjustment -= currentAdjustment;
        return currentAdjustment;
    }
}

void Network::initTimer()
{
    tickTimer.updateSavedTime();
}

void Network::registerCurrentTickPtr(
    const std::atomic<Uint32>* inCurrentTickPtr)
{
    currentTickPtr = inCurrentTickPtr;
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
        if (!(npcUpdateQueue.enqueue({NpcUpdateType::ExplicitConfirmation}))) {
            LOG_ERROR("Ran out of room in queue and memory allocation failed.");
        }
    }

    // Record the number of received bytes.
    NetworkStats::recordBytesReceived(bytesReceived);
}

void Network::processReceivedMessage(MessageType messageType,
                                     Uint16 messageSize)
{
    /* Funnel the message into the appropriate queue. */
    if (messageType == MessageType::ConnectionResponse) {
        // Deserialize the message.
        std::unique_ptr<ConnectionResponse> connectionResponse
            = std::make_unique<ConnectionResponse>();
        MessageTools::deserialize(messageRecBuffer, messageSize,
                                  *connectionResponse);

        // Grab our player ID so we can determine which update messages are for
        // the player.
        playerID = connectionResponse->entityID;

        // Queue the message.
        if (!(connectionResponseQueue.enqueue(std::move(connectionResponse)))) {
            LOG_ERROR("Ran out of room in queue and memory allocation failed.");
        }
    }
    else if (messageType == MessageType::EntityUpdate) {
        // Deserialize the message.
        std::shared_ptr<EntityUpdate> entityUpdate
            = std::make_shared<EntityUpdate>();
        MessageTools::deserialize(messageRecBuffer, messageSize, *entityUpdate);

        // Pull out the vector of entities.
        const std::vector<Entity>& entities = entityUpdate->entities;

        // Iterate through the entities, checking if there's player or npc data.
        bool playerFound = false;
        bool npcFound = false;
        for (auto entityIt = entities.begin(); entityIt != entities.end();
             ++entityIt) {
            EntityID entityID = entityIt->id;

            if (entityID == playerID) {
                // Found the player.
                if (!(playerUpdateQueue.enqueue(entityUpdate))) {
                    LOG_ERROR("Ran out of room in queue and memory allocation "
                              "failed.");
                }
                playerFound = true;
            }
            else if (!npcFound) {
                // Found a non-player (npc).
                // Queueing the message will let all npc updates within be
                // processed.
                if (!(npcUpdateQueue.enqueue(
                        {NpcUpdateType::Update, entityUpdate}))) {
                    LOG_ERROR("Ran out of room in queue and memory allocation "
                              "failed.");
                }
                npcFound = true;
            }

            // If we found the player and an npc, we can stop looking.
            if (playerFound && npcFound) {
                break;
            }
        }

        // If we didn't find an NPC and queue an update message, push an
        // implicit confirmation to show that we've confirmed up to this tick.
        if (!npcFound) {
            if (!(npcUpdateQueue.enqueue({NpcUpdateType::ImplicitConfirmation,
                                          nullptr, entityUpdate->tickNum}))) {
                LOG_ERROR(
                    "Ran out of room in queue and memory allocation failed.");
            }
        }
    }
}

void Network::adjustIfNeeded(Sint8 receivedTickAdj, Uint8 receivedAdjIteration)
{
    if (receivedTickAdj != 0) {
        Uint8 currentAdjIteration = adjustmentIteration;

        // If we haven't already processed this adjustment iteration.
        if (receivedAdjIteration == currentAdjIteration) {
            // Apply the adjustment.
            tickAdjustment += receivedTickAdj;

            // Increment the iteration.
            adjustmentIteration = (currentAdjIteration + 1);
            LOG_INFO("Received tick adjustment: %d, iteration: %u",
                     receivedTickAdj, receivedAdjIteration);
        }
        else if (receivedAdjIteration > currentAdjIteration) {
            LOG_ERROR("Out of sequence adjustment iteration. current: %u, "
                      "received: %u",
                      currentAdjIteration, receivedAdjIteration);
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
