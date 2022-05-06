#include "Network.h"
#include "Acceptor.h"
#include "Peer.h"
#include "Deserialize.h"
#include "Heartbeat.h"
#include "Log.h"
#include "NetworkStats.h"
#include <SDL_net.h>
#include <algorithm>
#include <atomic>
#include "Tracy.hpp"

namespace AM
{
namespace Server
{

Network::Network(EventDispatcher& inNetworkEventDispatcher)
: messageProcessor(inNetworkEventDispatcher)
, clientHandler(*this, inNetworkEventDispatcher, messageProcessor)
, ticksSinceNetstatsLog(0)
, currentTickPtr(nullptr)
{
}

void Network::tick()
{
    // Flag the send thread to start sending all messages for this network
    // tick.
    clientHandler.beginSendClientUpdates();

    // If it's time to log our network statistics, do so.
    ticksSinceNetstatsLog++;
    if (ticksSinceNetstatsLog == TICKS_TILL_STATS_DUMP) {
        logNetworkStatistics();
        ticksSinceNetstatsLog = 0;
    }
}

ClientMap& Network::getClientMap()
{
    return clientMap;
}

SharedLockableBase(std::shared_mutex) & Network::getClientMapMutex()
{
    return clientMapMutex;
}

void Network::registerCurrentTickPtr(
    const std::atomic<Uint32>* inCurrentTickPtr)
{
    currentTickPtr = inCurrentTickPtr;
}

Uint32 Network::getCurrentTick()
{
    return *currentTickPtr;
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

void Network::logNetworkStatistics()
{
    // Dump the stats from the tracker.
    NetStatsDump netStats{NetworkStats::dumpStats()};

    // Log the stats.
    float bytesSentPerSecond{netStats.bytesSent
                             / static_cast<float>(SECONDS_TILL_STATS_DUMP)};
    float bytesReceivedPerSecond{netStats.bytesReceived
                                 / static_cast<float>(SECONDS_TILL_STATS_DUMP)};
    LOG_INFO("Bytes sent per second: %.0f, Bytes received per second: %.0f",
             bytesSentPerSecond, bytesReceivedPerSecond);
}

} // namespace Server
} // namespace AM
