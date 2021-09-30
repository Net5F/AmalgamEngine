#include "ClientHandler.h"
#include "Network.h"
#include "NetworkDefs.h"
#include "SocketSet.h"
#include <shared_mutex>
#include <mutex>
#include <memory>
#include "Log.h"

namespace AM
{
namespace Server
{
ClientHandler::ClientHandler(Network& inNetwork, EventDispatcher& inDispatcher, MessageProcessor& inMessageProcessor)
: network{inNetwork}
, dispatcher{inDispatcher}
, messageProcessor{inMessageProcessor}
, idPool(MAX_CLIENTS)
, clientSet(std::make_shared<SocketSet>(MAX_CLIENTS))
, acceptor(Network::SERVER_PORT, clientSet)
, messageRecBuffer(Peer::MAX_MESSAGE_SIZE)
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
            numReceived = receiveAndProcessClientMessages(clientMap);
        }

        // If we received messages, process them.
        if (numReceived == 0) {
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

        {
            // Add the peer to the Network's clientMap.
            std::unique_lock writeLock(network.getClientMapMutex());
            if (!(clientMap
                      .try_emplace(newID, std::make_shared<Client>(
                                              newID, std::move(newPeer)))
                      .second)) {
                idPool.freeID(newID);
                LOG_ERROR("Ran out of room in client map or key already existed.");
            }
        }

        // Notify the sim that a client was connected.
        dispatcher.emplace<ClientConnected>(newID);

        newPeer = acceptor.accept();
    }
}

void ClientHandler::eraseDisconnectedClients(ClientMap& clientMap)
{
    /* Erase any disconnected clients. */
    for (auto it = clientMap.begin(); it != clientMap.end();) {
        std::shared_ptr<Client>& client = it->second;

        if (!(client->isConnected())) {
            // Save the ID since we're going to erase this client.
            NetworkID clientID{it->first};

            {
                // Need to modify the map, acquire a write lock.
                std::unique_lock writeLock(network.getClientMapMutex());

                // Erase the disconnected client.
                idPool.freeID(it->first);
                it = clientMap.erase(it);
            }

            // Notify the sim that a client was disconnected.
            LOG_INFO("Erased disconnected client with netID: %u.", clientID);
            dispatcher.emplace<ClientDisconnected>(clientID);
        }
        else {
            ++it;
        }
    }
}

int ClientHandler::receiveAndProcessClientMessages(ClientMap& clientMap)
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
        ReceiveResult result = clientPtr->receiveMessage(messageRecBuffer.data());
        while (result.networkResult == NetworkResult::Success) {
            numReceived++;

            // Process the message.
            processReceivedMessage(*clientPtr, result.messageType, result.messageSize);

            // Try to receive the next message.
            result = clientPtr->receiveMessage(messageRecBuffer.data());
        }
    }

    return numReceived;
}

void ClientHandler::processReceivedMessage(Client& client, MessageType messageType,
                                unsigned int messageSize)
{
    // Process the message.
    // Note: messageTick will be > -1 if the message contained a tick number.
    Sint64 messageTick = messageProcessor.processReceivedMessage(client.getNetID()
        , messageType, messageRecBuffer, messageSize);

    // If the message carried a tick number, use it to calc a diff and give it
    // to the client.
    if (messageTick != -1) {
        // Calc the difference between the current tick and the message's tick.
        Sint64 tickDiff{messageTick
            - static_cast<Sint64>(network.getCurrentTick())};

        // Record the diff.
        client.recordTickDiff(tickDiff);
    }
}

} // End namespace Server
} // End namespace AM
