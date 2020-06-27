#include "Network.h"
#include "Peer.h"
#include "NetworkHelpers.h"
#include <SDL2/SDL_net.h>
#include "Message_generated.h"
#include "Debug.h"

namespace AM
{
namespace Client
{

//const std::string Network::SERVER_IP = "127.0.0.1";
const std::string Network::SERVER_IP = "45.79.37.63";

Network::Network()
: server(nullptr)
, playerID(0)
, tickOffset(STARTING_TICK_OFFSET)
, adjustmentIteration(0)
, receiveThreadObj()
, exitRequested(false)
{
    SDLNet_Init();

    // Init the timer to the current time.
    receiveTimer.updateSavedTime();
}

Network::~Network()
{
    exitRequested = true;
    receiveThreadObj.join();
    SDLNet_Quit();
}

bool Network::connect()
{
    IPaddress ip;

    // Try to connect.
    server = Peer::initiate(SERVER_IP, SERVER_PORT);

    // Spin up the receive thread.
    if (server != nullptr) {
        receiveThreadObj = std::thread(&Network::pollForMessages, this);
    }

    return (server != nullptr);
}

void Network::registerPlayerID(EntityID inPlayerID)
{
    playerID = inPlayerID;
}

void Network::send(const BinaryBufferSharedPtr& message)
{
    if (!(server->isConnected())) {
        DebugError("Tried to send while server is disconnected.");
    }

    // Build the header.
    Uint8 header[CLIENT_HEADER_SIZE] = {adjustmentIteration};

    // Send the header.
    NetworkResult result = server->send(
        std::make_shared<BinaryBuffer>(header, header + CLIENT_HEADER_SIZE));
    if (result != NetworkResult::Success) {
        DebugError("Header send failed.");
    }

    // Send the message.
    result = server->send(message);
    if (result != NetworkResult::Success) {
        DebugError("Message send failed.");
    }
}

BinaryBufferSharedPtr Network::receive(MessageType type, Uint64 timeoutMs)
{
    if (!(server->isConnected())) {
        DebugInfo("Tried to receive while server is disconnected.");
        return nullptr;
    }

    MessageQueue* selectedQueue = nullptr;
    switch (type)
    {
        case (MessageType::ConnectionResponse):
            selectedQueue = &connectionResponseQueue;
            break;
        case (MessageType::PlayerUpdate):
            selectedQueue = &playerUpdateQueue;
            break;
        case (MessageType::NpcUpdate):
            selectedQueue = &npcUpdateQueue;
            break;
        default:
            DebugError("Provided unexpected type.");
    }

    BinaryBufferSharedPtr message = nullptr;
    if (timeoutMs == 0) {
        selectedQueue->try_dequeue(message);
    }
    else {
        selectedQueue->wait_dequeue_timed(message, timeoutMs * 1000);
    }

    return message;
}

int Network::pollForMessages()
{
    std::shared_ptr<Peer> server = getServer();
    std::atomic<bool> const* exitRequested = getExitRequestedPtr();

    while (!(*exitRequested)) {
        // Wait for a server header.
        ReceiveResult headerResult = server->receiveBytesWait(SERVER_HEADER_SIZE);

        if (headerResult.result == NetworkResult::Success) {
            const BinaryBuffer& header = *(headerResult.message.get());

            // Check if we need to apply a tick offset adjustment.
            Sint8 tickOffsetAdjustment =
                header[ServerHeaderIndex::TickOffsetAdjustment];

            // Check if we need to adjust the tick offset.
            // TODO: Move this to a function, or add a ServerHandler.
            if (tickOffsetAdjustment != 0) {
                Uint8 receivedAdjIteration =
                    header[ServerHeaderIndex::AdjustmentIteration];
                Uint8 currentAdjIteration = adjustmentIteration;

                // If we haven't already processed this adjustment iteration.
                if (receivedAdjIteration == currentAdjIteration) {
                    // Check that the adjustment is valid.
                    Uint8 currentOffset = tickOffset;
                    int adjustedOffset = currentOffset + tickOffsetAdjustment;
                    if ((adjustedOffset < 0) || (adjustedOffset > SDL_MAX_UINT8)) {
                        DebugError(
                            "Offset became invalid. currentOffset: %u, adjustedOffset: %d",
                            currentOffset, adjustedOffset);
                    }

                    // Apply the offset adjustment.
                    tickOffset = static_cast<Uint8>(adjustedOffset);

                    // Increment the iteration.
                    adjustmentIteration = (currentAdjIteration + 1);
                    DebugInfo("Applied tick adjustment: %d, for iteration: %u",
                        tickOffsetAdjustment, receivedAdjIteration);
                }
                else if (receivedAdjIteration > currentAdjIteration){
                    DebugError(
                        "Out of sequence adjustment iteration. current: %u, received: %u",
                        currentAdjIteration, receivedAdjIteration);
                }
            }

            // Receive all of the expected messages.
            Uint8 messageCount = header[ServerHeaderIndex::MessageCount];
            for (unsigned int i = 0; i < messageCount; ++i) {
                ReceiveResult messageResult = server->receiveMessageWait();

                // If we received a message, push it into the appropriate queue.
                if (messageResult.result == NetworkResult::Success) {
                    processReceivedMessage(std::move(messageResult.message));

                    // Got a message, update the receiveTimer.
                    receiveTimer.updateSavedTime();
                }
                else if ((messageResult.result == NetworkResult::NoWaitingData)
                && (receiveTimer.getDeltaSeconds(false) > TIMEOUT_S)) {
                    DebugError("Server connection timed out.");
                }
            }
        }
    }

    return 0;
}

void Network::processReceivedMessage(BinaryBufferPtr messageBuffer)
{
    // We might be sharing this message between queues, so convert it to shared.
    BinaryBufferSharedPtr sharedBuffer = std::move(messageBuffer);
    const fb::Message* message = fb::GetMessage(sharedBuffer->data());

    /* Funnel the message into the appropriate queue. */
    if (message->content_type() == fb::MessageContent::ConnectionResponse) {
        if (!(connectionResponseQueue.enqueue(sharedBuffer))) {
            DebugError("Ran out of room in queue and memory allocation failed.");
        }
    }
    else if (message->content_type() == fb::MessageContent::EntityUpdate) {
        // Pull out the vector of entities.
        auto entityUpdate = static_cast<const fb::EntityUpdate*>(message->content());
        auto entities = entityUpdate->entities();

        // Iterate through the entities, checking if there's player or npc data.
        bool playerFound = false;
        bool npcFound = false;
        for (auto entityIt = entities->begin(); entityIt != entities->end(); ++entityIt) {
            EntityID entityID = (*entityIt)->id();

            if (entityID == playerID) {
                // Found the player.
                if (!(playerUpdateQueue.enqueue(sharedBuffer))) {
                    DebugError("Ran out of room in queue and memory allocation failed.");
                    playerFound = true;
                }
            }
            else if (!npcFound){
                // Found a non-player (npc).
                // TODO: Add npc movement back in.
//                if (!(npcUpdateQueue.enqueue(sharedBuffer))) {
//                    DebugError("Ran out of room in queue and memory allocation failed.");
//                    npcFound = true;
//                }
            }

            // If we found the player and an npc, we can stop looking.
            if (playerFound && npcFound) {
                return;
            }
        }
    }
}

std::shared_ptr<Peer> Network::getServer()
{
    return server;
}

Uint8 Network::getTickOffset()
{
    return tickOffset;
}

void Network::recordTickOffset(Uint32 currentTick, Uint8 tickOffset)
{
    tickOffsetHistory.push({currentTick, tickOffset});
}

Uint8 Network::retrieveOffsetAtTick(Uint32 tickNum)
{
    TickOffsetMapping mapping = tickOffsetHistory.front();
    tickOffsetHistory.pop();
    if ((mapping.tickNum + mapping.tickOffset) != tickNum) {
        DebugError(
            "Recorded tick num doesn't match requested. "
                "Either send or receive must have happened out of order. "
                "Recorded: %u, offset: %d, requested: %u",
                mapping.tickNum, mapping.tickOffset, tickNum);
    }

    return mapping.tickOffset;
}

std::atomic<bool> const* Network::getExitRequestedPtr() {
    return &exitRequested;
}

} // namespace Client
} // namespace AM
