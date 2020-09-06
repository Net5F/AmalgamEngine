#include "Network.h"
#include "Peer.h"
#include <SDL2/SDL_net.h>
#include "Debug.h"

namespace AM
{
namespace Client
{

const std::string Network::SERVER_IP = "127.0.0.1";
//const std::string Network::SERVER_IP = "45.79.37.63";

Network::Network()
: server(nullptr)
, playerID(0)
, tickAdjustment(0)
, adjustmentIteration(0)
, receiveThreadObj()
, exitRequested(false)
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

void Network::registerPlayerID(EntityID inPlayerID)
{
    playerID = inPlayerID;
}

void Network::send(const BinaryBufferSharedPtr& message)
{
    if (!(server->isConnected())) {
        DebugError("Tried to send while server is disconnected.");
    }

    // Fill the message with the header (constructMessage() leaves
    // CLIENT_HEADER_SIZE bytes empty at the front for us to fill.)
    message->at(0) = adjustmentIteration;

    // Send the message.
    NetworkResult result = server->send(message);
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
            processHeader(header);
        }
    }

    return 0;
}

void Network::processHeader(const BinaryBuffer& header) {
    // Check if we need to adjust the tick offset.
    adjustIfNeeded(header[ServerHeaderIndex::TickAdjustment],
        header[ServerHeaderIndex::AdjustmentIteration]);

    /* Check if this is a batch or a heartbeat. */
    if ((header[ServerHeaderIndex::ConfirmedTickCount]
         & SERVER_HEARTBEAT_MASK) == 0) {
        /* Batch header, receive all of the expected messages. */
        Uint8 messageCount = header[ServerHeaderIndex::MessageCount];
        for (unsigned int i = 0; i < messageCount; ++i) {
            ReceiveResult messageResult = server->receiveMessageWait();

            // If we received a message, push it into the appropriate queue.
            if (messageResult.result == NetworkResult::Success) {
                // Got a message, process it and update the receiveTimer.
                processReceivedMessage(std::move(messageResult.message));
                receiveTimer.updateSavedTime();
            }
            else if ((messageResult.result == NetworkResult::NoWaitingData)
                     && (receiveTimer.getDeltaSeconds(false) > TIMEOUT_S)) {
                // Too long since we received a message, timed out.
                DebugError("Server connection timed out.");
            }
        }
    }
    else {
        /* Heartbeat, process the confirmed ticks. */
        Uint8 confirmedTickCount = header[ServerHeaderIndex::ConfirmedTickCount]
                                   ^ SERVER_HEARTBEAT_MASK;
        for (unsigned int i = 0; i < confirmedTickCount; ++i) {
            /* Construct a message with just the tick timestamp. */
            builder.Clear();
            fb::MessageBuilder messageBuilder(builder);
            messageBuilder.add_tickTimestamp(0);
            messageBuilder.add_content_type(fb::MessageContent::EntityUpdate);
            flatbuffers::Offset<fb::Message> message = messageBuilder.Finish();
            builder.Finish(message);

            BinaryBufferSharedPtr buffer = std::make_shared<BinaryBuffer>(
                builder.GetBufferPointer(),
                (builder.GetBufferPointer() + builder.GetSize()));
            npcUpdateQueue.enqueue(buffer);
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
                }
                playerFound = true;
            }
            else if (!npcFound){
                // Found a non-player (npc).
                // Queueing the message will let all npc updates within be processed.
                if (!(npcUpdateQueue.enqueue(sharedBuffer))) {
                    DebugError("Ran out of room in queue and memory allocation failed.");
                }
                npcFound = true;
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

std::atomic<bool> const* Network::getExitRequestedPtr() {
    return &exitRequested;
}

BinaryBufferSharedPtr Network::constructMessage(std::size_t size, Uint8* messageBuffer)
{
    if ((sizeof(Uint16) + size) > Peer::MAX_MESSAGE_SIZE) {
        DebugError("Tried to send a too-large message. Size: %u, max: %u", size,
            Peer::MAX_MESSAGE_SIZE);
    }

    // Allocate a buffer that can hold the header, the Uint16 size bytes, and the
    // message payload.
    // NOTE: We leave CLIENT_HEADER_SIZE bytes empty at the front of the message to be
    //       filled by the network before sending.
    BinaryBufferSharedPtr dynamicBuffer = std::make_shared<std::vector<Uint8>>(
        CLIENT_HEADER_SIZE + sizeof(Uint16) + size);

    // Copy the size into the buffer.
    _SDLNet_Write16(size, (dynamicBuffer->data() + CLIENT_HEADER_SIZE));

    // Copy the message into the buffer.
    std::copy(messageBuffer, messageBuffer + size,
        (dynamicBuffer->data() + CLIENT_HEADER_SIZE + sizeof(Uint16)));

    return dynamicBuffer;
}

} // namespace Client
} // namespace AM
