#include "Network.h"
#include "Peer.h"
#include "ConnectionResponse.h"
#include "EntityUpdate.h"
#include <SDL2/SDL_net.h>
#include "MessageTools.h"

namespace AM
{
namespace Client
{

Network::Network()
: server(nullptr)
, playerID(0)
, tickAdjustment(0)
, adjustmentIteration(0)
, receiveThreadObj()
, exitRequested(false)
, headerRecBuffer(SERVER_HEADER_SIZE)
, messageRecBuffer(Peer::MAX_MESSAGE_SIZE)
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

void Network::send(const BinaryBufferSharedPtr& message)
{
    if (!(server->isConnected())) {
        DebugError("Tried to send while server is disconnected.");
    }

    // Fill the message with the header (constructMessage() leaves
    // CLIENT_HEADER_SIZE bytes empty at the front for us to fill.)
    message->at(ClientHeaderIndex::AdjustmentIteration) = adjustmentIteration;

    // Send the message.
    NetworkResult result = server->send(message);
    if (result != NetworkResult::Success) {
        DebugError("Message send failed.");
    }
}

std::unique_ptr<ConnectionResponse> Network::receiveConnectionResponse(Uint64 timeoutMs)
{
    if (!(server->isConnected())) {
        DebugError("Tried to receive while server is disconnected.");
    }

    std::unique_ptr<ConnectionResponse> message = nullptr;
    if (timeoutMs == 0) {
        connectionResponseQueue.try_dequeue(message);
    }
    else {
        connectionResponseQueue.wait_dequeue_timed(message, timeoutMs * 1000);
    }

    return message;
}

std::shared_ptr<const EntityUpdate> Network::receivePlayerUpdate(Uint64 timeoutMs)
{
    if (!(server->isConnected())) {
        DebugError("Tried to receive while server is disconnected.");
    }

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
    if (!(server->isConnected())) {
        DebugInfo("Tried to receive while server is disconnected.");
        return {NetworkResult::Disconnected, {}};
    }

    NpcUpdateMessage message;
    bool messageWasReceived = false;
    if (timeoutMs == 0) {
        messageWasReceived = npcUpdateQueue.try_dequeue(message);
    }
    else {
        messageWasReceived = npcUpdateQueue.wait_dequeue_timed(message, timeoutMs * 1000);
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
    std::shared_ptr<Peer> server = getServer();
    std::atomic<bool> const* exitRequested = getExitRequestedPtr();

    while (!(*exitRequested)) {
        // Wait for a server header.
        NetworkResult headerResult = server->receiveBytesWait(headerRecBuffer.data(),
            SERVER_HEADER_SIZE);

        if (headerResult == NetworkResult::Success) {
            processBatch();
        }
        else if (headerResult == NetworkResult::Disconnected) {
            DebugError("Found server to be disconnected while trying to receive header.");
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
    else if (currentAdjustment == 0){
        return 0;
    }
    else {
        // The sim can process multiple iterations to catch up.
        tickAdjustment -= currentAdjustment;
        return currentAdjustment;
    }
}

void Network::processBatch() {
    // Check if we need to adjust the tick offset.
    adjustIfNeeded(headerRecBuffer[ServerHeaderIndex::TickAdjustment],
        headerRecBuffer[ServerHeaderIndex::AdjustmentIteration]);

    /* Process messages, if we received any. */
    Uint8 messageCount = headerRecBuffer[ServerHeaderIndex::MessageCount];
    for (unsigned int i = 0; i < messageCount; ++i) {
        MessageResult messageResult = server->receiveMessageWait(messageRecBuffer.data());

        // If we received a message, push it into the appropriate queue.
        if (messageResult.networkResult == NetworkResult::Success) {
            // Got a message, process it and update the receiveTimer.
            processReceivedMessage(messageResult.messageType, messageResult.messageSize);
            receiveTimer.updateSavedTime();
        }
        else if ((messageResult.networkResult == NetworkResult::NoWaitingData)
                 && (receiveTimer.getDeltaSeconds(false) > SERVER_TIMEOUT_S)) {
            // Too long since we received a message, timed out.
            DebugError("Server connection timed out.");
        }
        else if (messageResult.networkResult == NetworkResult::Disconnected) {
            DebugError("Found server to be disconnected while trying to receive message.");
        }
    }

    /* Process any confirmed ticks. */
    Uint8 confirmedTickCount = headerRecBuffer[ServerHeaderIndex::ConfirmedTickCount];
    for (unsigned int i = 0; i < confirmedTickCount; ++i) {
        if (!(npcUpdateQueue.enqueue({NpcUpdateType::ExplicitConfirmation}))) {
            DebugError("Ran out of room in queue and memory allocation failed.");
        }
    }
}

void Network::processReceivedMessage(MessageType messageType, Uint16 messageSize)
{
    /* Funnel the message into the appropriate queue. */
    if (messageType == MessageType::ConnectionResponse) {
        // Deserialize the message.
        std::unique_ptr<ConnectionResponse> connectionResponse
                                                = std::make_unique<ConnectionResponse>();
        MessageTools::deserialize(messageRecBuffer, messageSize, *connectionResponse);

        // Grab our player ID so we can determine what update messages are for the player.
        playerID = connectionResponse->entityID;

        // Queue the message.
        if (!(connectionResponseQueue.enqueue(std::move(connectionResponse)))) {
            DebugError("Ran out of room in queue and memory allocation failed.");
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
        for (auto entityIt = entities.begin(); entityIt != entities.end(); ++entityIt) {
            EntityID entityID = (*entityIt).id;

            if (entityID == playerID) {
                // Found the player.
                if (!(playerUpdateQueue.enqueue(entityUpdate))) {
                    DebugError("Ran out of room in queue and memory allocation failed.");
                }
                playerFound = true;
            }
            else if (!npcFound){
                // Found a non-player (npc).
                // Queueing the message will let all npc updates within be processed.
                if (!(npcUpdateQueue.enqueue({NpcUpdateType::Update, entityUpdate}))) {
                    DebugError("Ran out of room in queue and memory allocation failed.");
                }
                npcFound = true;
            }

            // If we found the player and an npc, we can stop looking.
            if (playerFound && npcFound) {
                break;
            }
        }

        // If we didn't find an NPC and queue an update message, push an implicit
        // confirmation to show that we've confirmed up to this tick.
        if (!npcFound) {
            if (!(npcUpdateQueue.enqueue({NpcUpdateType::ImplicitConfirmation, nullptr,
                    entityUpdate->tickNum}))) {
                DebugError("Ran out of room in queue and memory allocation failed.");
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
            DebugInfo("Received tick adjustment: %d, iteration: %u",
                receivedTickAdj, receivedAdjIteration);
        }
        else if (receivedAdjIteration > currentAdjIteration){
            DebugError(
                "Out of sequence adjustment iteration. current: %u, received: %u",
                currentAdjIteration, receivedAdjIteration);
        }
    }
}

std::shared_ptr<Peer> Network::getServer() const
{
    return server;
}

std::atomic<bool> const* Network::getExitRequestedPtr() const
{
    return &exitRequested;
}

} // namespace Client
} // namespace AM
