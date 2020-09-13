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

void Network::tick()
{
    accumulatedTime += sendTimer.getDeltaSeconds(true);

    if (accumulatedTime >= NETWORK_TICK_TIMESTEP_S) {
        // Send all messages for this network tick.
        sendClientUpdates();

        accumulatedTime -= NETWORK_TICK_TIMESTEP_S;
        if (accumulatedTime >= NETWORK_TICK_TIMESTEP_S) {
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

void Network::send(NetworkID networkID, const BinaryBufferSharedPtr& message)
{
    // Queue the message to be sent with the next batch.
    std::shared_lock readLock(clientMapMutex);

    // Check that the client still exists, queue the message if so.
    auto clientPair = clientMap.find(networkID);
    if (clientPair != clientMap.end()) {
        clientPair->second.queueMessage(message);
    }
}

void Network::sendToAll(const BinaryBufferSharedPtr& message)
{
    // Queue the message to be sent with the next batch.
    std::shared_lock readLock(clientMapMutex);
    for (auto& pair : clientMap) {
        pair.second.queueMessage(message);
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

void Network::initTimer()
{
    sendTimer.updateSavedTime();
}

void Network::sendClientUpdates()
{
    /* Run through the clients, sending their waiting messages. */
    std::shared_lock readLock(clientMapMutex);
    for (auto& pair : clientMap) {
        Client& client = pair.second;
        client.sendWaitingMessages(*currentTickPtr);
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

BinaryBufferSharedPtr Network::constructMessage(Uint8* messageBuffer,
                                                std::size_t size)
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
