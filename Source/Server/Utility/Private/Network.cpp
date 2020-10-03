#include "Network.h"
#include "Acceptor.h"
#include "Peer.h"
#include <SDL2/SDL_net.h>
#include "MessageTools.h"
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
, currentTickPtr(nullptr)
{
    tickTimer.updateSavedTime();
}

void Network::tick()
{
    accumulatedTime += tickTimer.getDeltaSeconds(true);

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

void Network::send(NetworkID networkID, const BinaryBufferSharedPtr& message,
                   Uint32 messageTick)
{
    // Queue the message to be sent with the next batch.
    std::shared_lock readLock(clientMapMutex);

    // Check that the client still exists, queue the message if so.
    auto clientPair = clientMap.find(networkID);
    if (clientPair != clientMap.end()) {
        clientPair->second->queueMessage(message, messageTick);
    }
}

void Network::processReceivedMessages(std::queue<ClientMessage>& receiveQueue)
{
    /* Process all messages in the queue. */
    while (!receiveQueue.empty()) {
        ClientMessage& clientMessage = receiveQueue.front();
        if (clientMessage.message.messageType == MessageType::ClientInputs) {
            DebugInfo("Received message was clientinputs.");
            // Deserialize the message.
            std::unique_ptr<ClientInputs> clientInputs = std::make_unique<ClientInputs>();
            BinaryBufferPtr& messageBuffer = clientMessage.message.messageBuffer;
            MessageTools::deserialize(*messageBuffer, messageBuffer->size(),
                *clientInputs);

            // Push the message (blocks if the MessageSorter is locked).
            // Save the tickNum since the move might be optimized before the access.
            Uint32 messageTickNum = clientInputs->tickNum;
            MessageSorterBase::PushResult pushResult = inputMessageSorter.push(
                messageTickNum, std::move(clientInputs));

            // Record the diff.
            if (pushResult.result == MessageSorterBase::ValidityResult::Valid) {
                if (std::shared_ptr<Client> clientPtr = clientMessage.clientPtr.lock()) {
                    clientPtr->recordTickDiff(pushResult.diff);
                }
                // Else, the client was destructed so we don't care.
            }
            else {
                DebugInfo("Message was dropped. Diff: %d", pushResult.diff);
            }
        }
        else if (clientMessage.message.messageType == MessageType::Heartbeat) {
//            /* Received a heartbeat, just return the diff. */
//            // Calc how far ahead or behind the message's tick is in relation to the currentTick.
//            // Using the game's currentTick should be accurate since we didn't have to
//            // lock anything.
//            return static_cast<Sint64>(receivedTickTimestamp)
//                   - static_cast<Sint64>(*currentTickPtr);
        }
        else {
            DebugError("Received message type that we aren't handling.");
        }

        receiveQueue.pop();
    }
}

std::queue<std::unique_ptr<ClientInputs>>& Network::startReceiveInputMessages(
Uint32 tickNum)
{
    return inputMessageSorter.startReceive(tickNum);
}

void Network::endReceiveInputMessages()
{
    inputMessageSorter.endReceive();
}

void Network::initTimer()
{
    tickTimer.updateSavedTime();
}

void Network::sendClientUpdates()
{
    /* Run through the clients, sending their waiting messages. */
    std::shared_lock readLock(clientMapMutex);
    for (auto& pair : clientMap) {
        pair.second->sendWaitingMessages(*currentTickPtr);
    }
}

ClientMap& Network::getClientMap()
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

BinaryBufferSharedPtr Network::constructMessage(MessageType type, Uint8* messageBuffer,
                                                std::size_t size)
{
    if ((MESSAGE_HEADER_SIZE + size) > Peer::MAX_MESSAGE_SIZE) {
        DebugError("Tried to send a too-large message. Size: %u, max: %u", size,
            Peer::MAX_MESSAGE_SIZE);
    }

    // Allocate a buffer that can hold the Uint8 type, Uint16 size, and the payload.
    BinaryBufferSharedPtr dynamicBuffer = std::make_shared<std::vector<Uint8>>(
        MESSAGE_HEADER_SIZE + size);

    // Copy the type into the buffer.
    dynamicBuffer->at(0) = static_cast<Uint8>(type);

    // Copy the size into the buffer.
    _SDLNet_Write16(size, dynamicBuffer->data());

    // Copy the message into the buffer.
    std::copy(messageBuffer, (messageBuffer + size),
        MESSAGE_HEADER_SIZE + dynamicBuffer->data());

    return dynamicBuffer;
}

} // namespace Server
} // namespace AM
