#include "Network.h"
#include "Acceptor.h"
#include "Peer.h"
#include "MessageTools.h"
#include "Heartbeat.h"
#include "Log.h"
#include "NetworkStats.h"
#include <SDL2/SDL_net.h>
#include <algorithm>
#include <atomic>

namespace AM
{
namespace Server
{
Network::Network()
: accumulatedTime(0.0)
, clientHandler(*this)
, ticksSinceNetstatsLog(0)
, currentTickPtr(nullptr)
{
    // Init the timer to the current time.
    tickTimer.updateSavedTime();
}

void Network::tick()
{
    accumulatedTime += tickTimer.getDeltaSeconds(true);

    if (accumulatedTime >= NETWORK_TICK_TIMESTEP_S) {
        // Send all messages for this network tick.
        sendClientUpdates();

        // If it's time to log our network statistics, do so.
        ticksSinceNetstatsLog++;
        if (ticksSinceNetstatsLog == TICKS_TILL_STATS_DUMP) {
            logNetworkStatistics();
            ticksSinceNetstatsLog = 0;
        }

        accumulatedTime -= NETWORK_TICK_TIMESTEP_S;
        if (accumulatedTime >= NETWORK_TICK_TIMESTEP_S) {
            // If we've accumulated enough time to send more, something
            // happened to delay us.
            // We still only want to send what's in the queue, but it's worth
            // giving debug output that we detected this.
            LOG_INFO("Detected a delayed network send. accumulatedTime: %f. "
                     "Setting to 0.",
                     accumulatedTime);
            accumulatedTime = 0;
        }
    }
}

void Network::send(NetworkID networkID, const BinaryBufferSharedPtr& message,
                   Uint32 messageTick)
{
    // Acquire a read lock before running through the client map.
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
        BinaryBufferPtr& messageBuffer = clientMessage.message.messageBuffer;

        // Used for recording how far ahead or behind the client's tick is.
        Sint64 tickDiff = 0;

        // Process the message.
        switch (clientMessage.message.messageType) {
            case MessageType::ClientInputs: {
                tickDiff = handleClientInputs(clientMessage, messageBuffer);
                break;
            }
            case MessageType::Heartbeat: {
                tickDiff = handleHeartbeat(messageBuffer);
                break;
            }
            default: {
                LOG_ERROR("Received message type that we aren't handling.");
                break;
            }
        }

        // Record the diff.
        if (std::shared_ptr<Client> clientPtr
            = clientMessage.clientPtr.lock()) {
            clientPtr->recordTickDiff(tickDiff);
        }
        // Else, the client was destructed so we don't care.

        receiveQueue.pop();
    }
}

std::queue<std::unique_ptr<ClientInputs>>&
    Network::startReceiveInputMessages(Uint32 tickNum)
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

moodycamel::ReaderWriterQueue<NetworkID>& Network::getMessageDropEventQueue()
{
    return messageDropEventQueue;
}

void Network::registerCurrentTickPtr(
    const std::atomic<Uint32>* inCurrentTickPtr)
{
    currentTickPtr = inCurrentTickPtr;
}

BinaryBufferSharedPtr Network::constructMessage(MessageType type,
                                                Uint8* messageBuffer,
                                                std::size_t size)
{
    if ((MESSAGE_HEADER_SIZE + size) > Peer::MAX_MESSAGE_SIZE) {
        LOG_ERROR("Tried to send a too-large message. Size: %u, max: %u", size,
                  Peer::MAX_MESSAGE_SIZE);
    }

    // Allocate a buffer that can hold the Uint8 type, Uint16 size, and the
    // payload.
    BinaryBufferSharedPtr dynamicBuffer
        = std::make_shared<std::vector<Uint8>>(MESSAGE_HEADER_SIZE + size);

    // Copy the type into the buffer.
    dynamicBuffer->at(0) = static_cast<Uint8>(type);

    // Copy the size into the buffer.
    _SDLNet_Write16(size, dynamicBuffer->data());

    // Copy the message into the buffer.
    std::copy(messageBuffer, (messageBuffer + size),
              MESSAGE_HEADER_SIZE + dynamicBuffer->data());

    return dynamicBuffer;
}

void Network::sendClientUpdates()
{
    // Acquire a read lock before running through the client map.
    std::shared_lock readLock(clientMapMutex);

    // Run through the clients, sending their waiting messages.
    for (auto& pair : clientMap) {
        pair.second->sendWaitingMessages(*currentTickPtr);
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

Sint64 Network::handleClientInputs(ClientMessage& clientMessage, BinaryBufferPtr& messageBuffer)
{
    // Deserialize the message.
    std::unique_ptr<ClientInputs> clientInputs
        = std::make_unique<ClientInputs>();
    MessageTools::deserialize(*messageBuffer, messageBuffer->size(),
                              *clientInputs);

    // Fill in the network ID that we assigned to this client.
    clientInputs->netID = clientMessage.netID;

    // Push the message (blocks if the MessageSorter is locked).
    // Save the tickNum locally since the move might be optimized
    // in front of the access (this does happen).
    Uint32 messageTickNum = clientInputs->tickNum;
    MessageSorterBase::PushResult pushResult
        = inputMessageSorter.push(messageTickNum,
                                  std::move(clientInputs));

    // If the sorter dropped the message, push a message drop event.
    if (pushResult.result
        != MessageSorterBase::ValidityResult::Valid) {
        if (!messageDropEventQueue.enqueue(clientMessage.netID)) {
            LOG_ERROR("Ran out of room in queue and memory allocation failed.");
        }

        LOG_INFO(
            "Message was dropped. NetID: %u, diff: %d, result: %u, "
            "tickNum: %u",
            clientMessage.netID, pushResult.diff, pushResult.result,
            messageTickNum);
    }

    // Save the diff that the MessageSorter returned.
    return pushResult.diff;
}

Sint64 Network::handleHeartbeat(BinaryBufferPtr& messageBuffer)
{
    // Deserialize the message.
    Heartbeat heartbeat{};
    MessageTools::deserialize(*messageBuffer, messageBuffer->size(),
                              heartbeat);

    // Calc the diff. Using the game's currentTick should be
    // accurate since we didn't have to lock anything.
    return static_cast<Sint64>(heartbeat.tickNum) - static_cast<Sint64>(*currentTickPtr);
}

} // namespace Server
} // namespace AM
