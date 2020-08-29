#include "Network.h"
#include "Acceptor.h"
#include "Peer.h"
#include <SDL2/SDL_net.h>
#include <algorithm>
#include <atomic>
#include "Debug.h"

namespace AM
{
namespace Server
{

Network::Network()
: accumulatedTime(0.0)
, clientHandler(*this)
, builder(BUILDER_BUFFER_SIZE)
, currentTickPtr(nullptr)
{
    sendTimer.updateSavedTime();
}

void Network::send(NetworkID id, const BinaryBufferSharedPtr& message)
{
    // Queue the message to be sent with the next batch.
    std::shared_lock readLock(clientMapMutex);
    clientMap.at(id).queueMessage(message);
}

void Network::sendToAll(const BinaryBufferSharedPtr& message)
{
    // Queue the message to be sent with the next batch.
    std::shared_lock readLock(clientMapMutex);
    for (auto& pair : clientMap) {
        pair.second.queueMessage(message);
    }
}

void Network::sendWaitingMessages()
{
    accumulatedTime += sendTimer.getDeltaSeconds(true);

    if (accumulatedTime >= NETWORK_TICK_INTERVAL_S) {
        // Queue connection responses before starting to send this batch.
        queueConnectionResponses();

        // Send all messages for this batch.
        sendWaitingMessagesInternal();

        accumulatedTime -= NETWORK_TICK_INTERVAL_S;
        if (accumulatedTime >= NETWORK_TICK_INTERVAL_S) {
            // If we've accumulated enough time to send more, something
            // happened to delay us.
            // We still only want to send what's in the queue, but it's worth giving
            // debug output that we detected this.
            DebugInfo(
                "Detected a delayed network send. accumulatedTime: %f. Setting to 0.",
                accumulatedTime);
            accumulatedTime = 0;
        }
    }
}

Sint64 Network::queueInputMessage(BinaryBufferPtr messageBuffer)
{
    const fb::Message* message = fb::GetMessage(messageBuffer->data());
    if (message->content_type() != fb::MessageContent::EntityUpdate) {
        DebugError("Expected EntityUpdate but got something else.");
    }
    Uint32 receivedTickTimestamp = message->tickTimestamp();

    // Check if the message is just a heartbeat, or if we need to push it.
    auto entityUpdate = static_cast<const fb::EntityUpdate*>(message->content());
    if (entityUpdate->entities()->size() != 0) {
        /* Received a message, try to push it. */
        MessageSorter::PushResult pushResult = inputMessageSorter.push(receivedTickTimestamp,
            std::move(messageBuffer));
        if (pushResult.result != MessageSorter::ValidityResult::Valid) {
            DebugInfo("Message was dropped. Diff: %d", pushResult.diff);
        }

        return pushResult.diff;
    }
    else {
        /* Received a heartbeat, just return the diff. */
        // Calc how far ahead or behind the message's tick is in relation to the currentTick.
        // Using the game's currentTick should be accurate since we didn't have to
        // lock anything.
        return static_cast<Sint64>(receivedTickTimestamp)
               - static_cast<Sint64>(*currentTickPtr);
    }
}

std::queue<BinaryBufferPtr>& Network::startReceiveInputMessages(Uint32 tickNum)
{
    return inputMessageSorter.startReceive(tickNum);
}

void Network::endReceiveInputMessages()
{
    inputMessageSorter.endReceive();
}

void Network::sendConnectionResponse(NetworkID networkID, EntityID newEntityID,
                                     float spawnX, float spawnY)
{
    std::shared_lock readLock(clientMapMutex);
    if (clientMap.find(networkID) == clientMap.end()) {
        // Client disconnected or bug is present.
        DebugInfo(
            "Tried to send a connectionResponse to a client that isn't in the clients map.")
    }

    connectionResponseQueue.push({networkID, newEntityID, spawnX, spawnY});
}

void Network::initTimer()
{
    sendTimer.updateSavedTime();
}

void Network::queueConnectionResponses()
{
    /* Send all waiting ConnectionResponse messages. */
    unsigned int responseCount = connectionResponseQueue.size();
    if (responseCount > 0) {
        Uint32 latestTickTimestamp = *currentTickPtr;

        for (unsigned int i = 0; i < responseCount; ++i) {
            ConnectionResponseData& data = connectionResponseQueue.front();

            // Prep the builder for a new message.
            builder.Clear();

            // Send them their ID and spawn point.
            auto response = fb::CreateConnectionResponse(builder, data.entityID,
                data.spawnX, data.spawnY);
            auto encodedMessage = fb::CreateMessage(builder, latestTickTimestamp,
                fb::MessageContent::ConnectionResponse, response.Union());
            builder.Finish(encodedMessage);

            // Queue the message so it gets included in the next batch.
            send(data.networkID,
                constructMessage(builder.GetSize(), builder.GetBufferPointer()));
            DebugInfo("Sent new client response with tick = %u", latestTickTimestamp);

            connectionResponseQueue.pop();
        }
    }
}

void Network::sendWaitingMessagesInternal()
{
    /* Send the waiting messages in every client's queue. */
    std::shared_lock readLock(clientMapMutex);
    for (auto& pair : clientMap) {
        Client& client = pair.second;

        Uint8 messageCount = client.getWaitingMessageCount();
        if (messageCount > 0) {
            // Build the batch header.
            Client::AdjustmentData tickAdjustment = client.getTickAdjustment();
            Uint8 header[SERVER_HEADER_SIZE] = {
                    static_cast<Uint8>(tickAdjustment.adjustment),
                    tickAdjustment.iteration, messageCount };

            // Send the batch header.
            client.sendHeader(
                std::make_shared<BinaryBuffer>(header,
                    header + SERVER_HEADER_SIZE));

            // Send all waiting messages.
            client.sendWaitingMessages();
        }
    }
}

std::unordered_map<NetworkID, Client>& Network::getClientMap()
{
    return clientMap;
}

std::shared_mutex& Network::getClientMapMutex()
{
    return clientMapMutex;
}

moodycamel::ReaderWriterQueue<NetworkID>& Network::getConnectEventQueue()
{
    return connectEventQueue;
}

moodycamel::ReaderWriterQueue<NetworkID>& Network::getDisconnectEventQueue()
{
    return disconnectEventQueue;
}

void Network::registerCurrentTickPtr(const std::atomic<Uint32>* inCurrentTickPtr)
{
    currentTickPtr = inCurrentTickPtr;
}

BinaryBufferSharedPtr Network::constructMessage(std::size_t size,
                                                Uint8* messageBuffer) const
{
    if ((sizeof(Uint16) + size) > Peer::MAX_MESSAGE_SIZE) {
        DebugError("Tried to send a too-large message. Size: %u, max: %u", size,
            Peer::MAX_MESSAGE_SIZE);
    }

    // Allocate a buffer that can hold the Uint16 size bytes and the message payload.
    BinaryBufferSharedPtr dynamicBuffer = std::make_shared<std::vector<Uint8>>(
        sizeof(Uint16) + size);

    // Copy the size into the buffer.
    _SDLNet_Write16(size, dynamicBuffer->data());

    // Copy the message into the buffer.
    std::copy(messageBuffer, messageBuffer + size,
        dynamicBuffer->data() + sizeof(Uint16));

    return dynamicBuffer;
}

} // namespace Server
} // namespace AM
