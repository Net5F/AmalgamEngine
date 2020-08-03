#include "ClientHandler.h"
#include "Network.h"
#include <shared_mutex>
#include <mutex>
#include "Debug.h"

namespace AM
{
namespace Server
{

ClientHandler::ClientHandler(Network& inNetwork)
: network(inNetwork)
, idPool()
, acceptor(nullptr)
, clientSet(std::make_shared<SDLNet_SocketSet>(SDLNet_AllocSocketSet(MAX_CLIENTS)))
, receiveThreadObj()
, exitRequested(false)
{
    SDLNet_Init();

    // We delay creating the acceptor because SDLNet_Init has to be called first.
    acceptor = std::make_unique<Acceptor>(Network::SERVER_PORT, clientSet);

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
    std::unique_ptr<Peer> newPeer = acceptor->accept();

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

        newPeer = acceptor->accept();
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
    int numReady = SDLNet_CheckSockets(*clientSet, 100);
    if (numReady == -1) {
        DebugInfo("Error while checking sockets: %s", SDLNet_GetError());
        // Most of the time this is a system error, where perror might help.
        perror("SDLNet_CheckSockets");
    }
    else if (numReady > 0) {
        /* Iterate through all clients. */
        // Doesn't need a lock because serviceClients is still locking.
        for (auto& pair : clientMap) {
            Client& client = pair.second;

            /* Receive all messages from the client. */
            ReceiveResult messageResult = client.receiveMessage();
            while (messageResult.result == NetworkResult::Success) {
                // Queue the message (blocks if the queue is locked).
                Sint64 diff = network.queueInputMessage(std::move(messageResult.message));
                client.recordTickDiff(diff);

                DebugInfo("Diff: %d", diff);

                messageResult = client.receiveMessage();
            }
        }
    }
}

} // End namespace Server
} // End namespace AM
