#include "ClientHandler.h"
#include "Network.h"
#include "SocketSet.h"
#include <shared_mutex>
#include <mutex>
#include <memory>
#include "Debug.h"

namespace AM
{
namespace Server
{

ClientHandler::ClientHandler(Network& inNetwork)
: network(inNetwork)
, idPool()
, clientSet(std::make_shared<SocketSet>(MAX_CLIENTS))
, acceptor(Network::SERVER_PORT, clientSet)
, receiveThreadObj()
, exitRequested(false)
{
    // Start the receive thread.
    receiveThreadObj = std::thread(&ClientHandler::serviceClients, this);
}

ClientHandler::~ClientHandler()
{
    exitRequested = true;
    receiveThreadObj.join();
    SDLNet_Quit();
}

int ClientHandler::serviceClients()
{
    std::unordered_map<EntityID, Client>& clientMap = network.getClientMap();

    while (!exitRequested) {
        // Check if there are any new clients to connect.
        acceptNewClients(clientMap);

        // Erase any clients who were detected to be disconnected.
        eraseDisconnectedClients(clientMap);

        // Check if there's any clients to receive from.
        std::shared_lock readLock(network.getClientMapMutex());
        int numActive = 0;
        if (clientMap.size() != 0) {
            numActive = receiveClientMessages(clientMap);
        }

        // Release the lock before we delay.
        readLock.unlock();

        // If there wasn't any activity, delay so we don't waste CPU spinning.
        if (numActive == 0) {
            SDL_Delay(INACTIVE_DELAY_TIME_MS);
        }
    }

    return 0;
}

void ClientHandler::acceptNewClients(
std::unordered_map<NetworkID, Client>& clientMap)
{
    // Creates the new peer, which adds itself to the socket set.
    std::unique_ptr<Peer> newPeer = acceptor.accept();

    while (newPeer != nullptr) {
        NetworkID newID = idPool.reserveID();
        DebugInfo("New client connected. Assigning netID: %u", newID);

        // Add the peer to the Network's clientMap, constructing a Client in-place.
        std::unique_lock writeLock(network.getClientMapMutex());
        if (!(clientMap.try_emplace(newID, newID, std::move(newPeer)).second)) {
            idPool.freeID(newID);
            DebugError("Ran out of room in client map or key already existed.");
        }

        // Add an event to the Network's queue.
        network.getConnectEventQueue().enqueue(newID);

        newPeer = acceptor.accept();
    }
}

void ClientHandler::eraseDisconnectedClients(
std::unordered_map<NetworkID, Client>& clientMap)
{
    std::shared_mutex& clientMapMutex = network.getClientMapMutex();

    /* Erase any disconnected clients. */
    // Only need a read lock to check for disconnects.
    std::shared_lock readLock(clientMapMutex);
    for (auto it = clientMap.begin(); it != clientMap.end();) {
        Client& client = it->second;

        if (!(client.isConnected())) {
            // Need to modify the map, acquire a write lock.
            readLock.unlock();
            std::unique_lock writeLock(clientMapMutex);

            // Add an event to the Network's queue.
            network.getDisconnectEventQueue().enqueue(it->first);

            // Erase the disconnected client.
            DebugInfo("Erased disconnected client with netID: %u.", it->first);
            idPool.freeID(it->first);
            it = clientMap.erase(it);
        }
        else {
            ++it;
        }
    }
}

int ClientHandler::receiveClientMessages(std::unordered_map<EntityID, Client>& clientMap)
{
    // Update each client's internal socket isReady().
    // Note: We check all clients regardless of whether this returns > 0 because, even if
    //       there's no activity, we need to check for timeouts.
    int numActive = clientSet->checkSockets(0);

    /* Iterate through all clients. */
    // Note: Doesn't need a lock because serviceClients is still locking.
    for (auto& pair : clientMap) {
        Client& client = pair.second;

        /* If there's potentially data, try to receive all messages from the client. */
        MessageResult messageResult = client.receiveMessage();
        while (messageResult.networkResult == NetworkResult::Success) {
            // TEMP
//                Uint32 receivedTick = 0;
//                const fb::Message* message = fb::GetMessage(messageResult.message->data());
//                if (message->content_type() == fb::MessageContent::EntityUpdate) {
//                    receivedTick = message->tickTimestamp();
//                }
            // TEMP

            // Queue the message (blocks if the queue is locked).
//            Sint64 diff = network.queueInputMessage(std::move(messageResult.message));
//            client.recordTickDiff(diff);

            // TEMP
//                if (receivedTick != 0) {
//                    DebugInfo("Received message for tick: %u. Diff: %d", receivedTick,
//                        diff);
//                }
//                else {
//                    DebugInfo("Received message. Diff: %d", diff);
//                }
            // TEMP

            messageResult = client.receiveMessage();
        }
    }

    return numActive;
}

} // End namespace Server
} // End namespace AM
