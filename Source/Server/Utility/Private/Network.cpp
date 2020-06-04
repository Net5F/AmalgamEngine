#include "Network.h"
#include "Acceptor.h"
#include "Peer.h"
#include <SDL2/SDL_net.h>
#include "NetworkHelpers.h"
#include <algorithm>
#include "Debug.h"

namespace AM
{
namespace Server
{

Uint32* Network::currentTickPtr = nullptr;

void Network::registerCurrentTickPtr(Uint32* inCurrentTickPtr)
{
    currentTickPtr = inCurrentTickPtr;
}

Network::Network()
: accumulatedTime(0.0f)
, builder(BUILDER_BUFFER_SIZE)
{
}

void Network::send(NetworkID id, BinaryBufferSharedPtr message)
{
    // Queue the message to be sent with the next batch.
    std::shared_lock readLock(clientMapMutex);
    clientMap.at(id).queueMessage(message);
}

void Network::sendToAll(BinaryBufferSharedPtr message)
{
    // Queue the message to be sent with the next batch.
    std::shared_lock readLock(clientMapMutex);
    for (auto& pair : clientMap) {
        pair.second.queueMessage(message);
    }
}

void Network::sendWaitingMessages(float deltaSeconds)
{
    accumulatedTime += deltaSeconds;

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

void Network::queueInputMessage(BinaryBufferSharedPtr messageBuffer)
{
    const fb::Message* message = fb::GetMessage(messageBuffer->data());
    if (message->content_type() != fb::MessageContent::EntityUpdate) {
        DebugError("Expected EntityUpdate but got something else.");
    }
    auto entityUpdate = static_cast<const fb::EntityUpdate*>(message->content());

    DebugInfo("Received message with tick: %u", message->tickTimestamp());

    // Push the message into the MessageSorter.
    bool result = inputMessageSorter.push(message->tickTimestamp(), messageBuffer);
    if (!result) {
        DebugInfo("Message rejected from MessageSorter.");
    }
}

std::queue<BinaryBufferSharedPtr>& Network::startReceiveInputMessages(Uint32 tickNum)
{
    return inputMessageSorter.startReceive(tickNum);
}

void Network::endReceiveInputMessages()
{
    inputMessageSorter.endReceive();
}

void Network::sendConnectionResponse(NetworkID id, float spawnX, float spawnY)
{
    std::shared_lock readLock(clientMapMutex);
    if (clientMap.find(id) == clientMap.end()) {
        DebugError(
            "Tried to send a connectionResponse to a client that isn't in the clients map.")
    }

    connectionResponseQueue.push({id, spawnX, spawnY});
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
            auto response = fb::CreateConnectionResponse(builder, data.id, data.spawnX,
                data.spawnY);
            auto encodedMessage = fb::CreateMessage(builder, latestTickTimestamp,
                fb::MessageContent::ConnectionResponse, response.Union());
            builder.Finish(encodedMessage);

            // Queue the message so it gets included in the next batch.
            send(data.id,
                NetworkHelpers::constructMessage(builder.GetSize(),
                    builder.GetBufferPointer()));
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
            Sint8 offset = -42;
            Uint8 header[SERVER_HEADER_SIZE] = {(Uint8) offset, messageCount};

            // Send the batch header.
            client.sendHeader(
                std::make_shared<std::vector<Uint8>>(header,
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

} // namespace Server
} // namespace AM
