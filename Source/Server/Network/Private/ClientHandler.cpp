#include "ClientHandler.h"
#include "Network.h"
#include "SocketSet.h"
#include <shared_mutex>
#include <mutex>
#include <memory>
#include "Log.h"

namespace AM
{
namespace Server
{
ClientHandler::ClientHandler(Network& inNetwork)
: network(inNetwork)
, idPool(MAX_CLIENTS)
, clientSet(std::make_shared<SocketSet>(MAX_CLIENTS))
, acceptor(Network::SERVER_PORT, clientSet)
, receiveThreadObj()
, exitRequested(false)
, sendRequested(false)
{
    // Start the send and receive threads.
    receiveThreadObj = std::thread(&ClientHandler::serviceClients, this);
    sendThreadObj = std::thread(&ClientHandler::sendClientUpdates, this);
}

ClientHandler::~ClientHandler()
{
    exitRequested = true;
    receiveThreadObj.join();

    {
        std::unique_lock<std::mutex> lock(sendMutex);
        sendRequested = true;
    }
    sendCondVar.notify_one();
    sendThreadObj.join();
}

void ClientHandler::beginSendClientUpdates()
{
    // Wake the send thread.
    {
        std::unique_lock<std::mutex> lock(sendMutex);
        sendRequested = true;
    }
    sendCondVar.notify_one();
}

void ClientHandler::serviceClients()
{
    ClientMap& clientMap = network.getClientMap();

    while (!exitRequested) {
        // Check if there are any new clients to connect.
        acceptNewClients(clientMap);

        // Erase any clients who were detected to be disconnected.
        eraseDisconnectedClients(clientMap);

        // Check if there's any clients with activity, and receive all their
        // messages.
        // Note: Doesn't need a lock because we only mutate the map from this
        //       thread.
        int numReceived = 0;
        if (clientMap.size() != 0) {
            numReceived = receiveClientMessages(clientMap);
        }

        // If we received messages, deserialize and route them.
        if (numReceived != 0) {
            network.processReceivedMessages(receiveQueue);
        }
        else {
            // There wasn't any activity, delay so we don't waste CPU spinning.
            SDL_Delay(INACTIVE_DELAY_TIME_MS);
        }
    }
}

void ClientHandler::sendClientUpdates()
{
    std::shared_mutex& clientMapMutex = network.getClientMapMutex();
    ClientMap& clientMap = network.getClientMap();

    while (!exitRequested) {
        // Wait until this thread is signaled by beginSendClientUpdates().
        std::unique_lock<std::mutex> lock(sendMutex);
        sendCondVar.wait(lock, [this] { return sendRequested; });

        // Acquire a read lock before running through the client map.
        std::shared_lock readLock(clientMapMutex);

        // Run through the clients, sending their waiting messages.
        Uint32 currentTick = network.getCurrentTick();
        for (auto& pair : clientMap) {
            pair.second->sendWaitingMessages(currentTick);
        }

        sendRequested = false;
    }
}

void ClientHandler::acceptNewClients(ClientMap& clientMap)
{
    // Creates the new peer, which adds itself to the socket set.
    std::unique_ptr<Peer> newPeer = acceptor.accept();

    while (newPeer != nullptr) {
        NetworkID newID = idPool.reserveID();
        LOG_INFO("New client connected. Assigning netID: %u", newID);

        // Add the peer to the Network's clientMap, constructing a Client
        // in-place.
        std::unique_lock writeLock(network.getClientMapMutex());
        if (!(clientMap
                  .try_emplace(newID, std::make_shared<Client>(
                                          newID, std::move(newPeer)))
                  .second)) {
            idPool.freeID(newID);
            LOG_ERROR("Ran out of room in client map or key already existed.");
        }

        // Add an event to the Network's queue.
        if (!network.getConnectEventQueue().enqueue(newID)) {
            LOG_ERROR("Ran out of room in queue and memory allocation failed.");
        }

        newPeer = acceptor.accept();
    }
}

void ClientHandler::eraseDisconnectedClients(ClientMap& clientMap)
{
    /* Erase any disconnected clients. */
    // Only need a read lock to check for disconnects.
    for (auto it = clientMap.begin(); it != clientMap.end();) {
        std::shared_ptr<Client>& client = it->second;

        if (!(client->isConnected())) {
            // Need to modify the map, acquire a write lock.
            std::unique_lock writeLock(network.getClientMapMutex());

            // Add an event to the Network's queue.
            network.getDisconnectEventQueue().enqueue(it->first);

            // Erase the disconnected client.
            LOG_INFO("Erased disconnected client with netID: %u.", it->first);
            idPool.freeID(it->first);
            it = clientMap.erase(it);
        }
        else {
            ++it;
        }
    }
}

int ClientHandler::receiveClientMessages(ClientMap& clientMap)
{
    // Update each client's internal socket isReady().
    // Note: We check all clients regardless of whether this returns > 0
    //       because, even if there's no activity, we need to check for
    //       timeouts.
    clientSet->checkSockets(0);

    /* Iterate through all clients. */
    // Note: Doesn't need a lock because we only mutate the map from this
    //       thread.
    int numReceived = 0;
    for (auto& pair : clientMap) {
        const std::shared_ptr<Client>& clientPtr = pair.second;

        /* If there's potentially data waiting, try to receive all messages
           from the client. */
        Message resultMessage = clientPtr->receiveMessage();
        while (resultMessage.messageType != MessageType::NotSet) {
            numReceived++;

            // Queue the message.
            std::weak_ptr<Client> clientWeakPtr = clientPtr;
            receiveQueue.emplace(clientPtr->getNetID(),
                                 std::move(clientWeakPtr),
                                 std::move(resultMessage));

            // Try to receive the next message.
            resultMessage = clientPtr->receiveMessage();
        }
    }

    return numReceived;
}

} // End namespace Server
} // End namespace AM
