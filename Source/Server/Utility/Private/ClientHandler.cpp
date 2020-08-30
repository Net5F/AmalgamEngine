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
        if (!(clientMap.emplace(newID, std::move(newPeer)).second)) {
            idPool.freeID(newID);
            DebugError(
                "Ran out of room in new client queue and memory allocation failed.");
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
        if (clientMap.size() == 0) {
            // Release the lock before we delay.
            readLock.unlock();

            // Delay so we don't waste CPU spinning.
            SDL_Delay(1);
        }
        else {
            receiveClientMessages(clientMap);
        }
    }

    return 0;
}

void ClientHandler::receiveClientMessages(std::unordered_map<EntityID, Client>& clientMap)
{
    // Check if there are any messages to receive.
    int numReady = clientSet->checkSockets(SOCKET_RECEIVE_TIMEOUT_MS);
    if (numReady > 0) {
        /* Iterate through all clients. */
        // Doesn't need a lock because serviceClients is still locking.
        for (auto& pair : clientMap) {
            Client& client = pair.second;

            /* Receive all messages from the client. */
            ReceiveResult messageResult = client.receiveMessage();
            while (messageResult.result == NetworkResult::Success) {
                // TEMP
                Uint32 receivedTick = 0;
                const fb::Message* message = fb::GetMessage(messageResult.message->data());
                if (message->content_type() == fb::MessageContent::EntityUpdate) {
                    receivedTick = message->tickTimestamp();
                }
                // TEMP

                // Queue the message (blocks if the queue is locked).
                Sint64 diff = network.queueInputMessage(std::move(messageResult.message));
                client.recordTickDiff(diff);

                // TEMP
                if (receivedTick != 0) {
                    DebugInfo("Received message for tick: %u. Diff: %d", receivedTick,
                        diff);
                }
                else {
                    DebugInfo("Received message. Diff: %d", diff);
                }
                // TEMP

                messageResult = client.receiveMessage();
            }
        }
    }
}

} // End namespace Server
} // End namespace AM
