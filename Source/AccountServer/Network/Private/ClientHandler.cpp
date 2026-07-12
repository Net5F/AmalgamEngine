#include "ClientHandler.h"
#include "SocketSet.h"
#include "Config.h"
#include "Log.h"

namespace AM
{
namespace AccountServer
{

ClientHandler::ClientHandler()
: clientCount{0}
, clientSet{std::make_shared<SocketSet>(Config::MAX_CLIENTS)}
, acceptor{Config::SERVER_PORT, clientSet}
{
}

void ClientHandler::processMessages()
{
    serviceClients();

    // TODO: This might end up not being used, we may do everything in 
    //       handlers
    sendClientUpdates();
}

void ClientHandler::serviceClients()
{
    // TODO: What kind of data will we need for each client?
    ClientMap& clientMap{network.getClientMap()};

    // Check if there are any new clients to connect.
    acceptNewClients(clientMap);

    // Erase any clients who were detected to be disconnected.
    eraseDisconnectedClients(clientMap);

    // Check if there's any clients with activity, and process all their
    // messages.
    // Note: Doesn't need a lock because we only mutate the map from this
    //       thread.
    if (clientMap.size() != 0) {
        receiveAndProcessClientMessages(clientMap);
    }
}

void ClientHandler::sendClientUpdates()
{
    ClientMap& clientMap{network.getClientMap()};

    // Run through the clients, sending their waiting messages.
    Uint32 currentTick{network.getCurrentTick()};
    for (auto& pair : clientMap) {
        pair.second->sendWaitingMessages(currentTick);
    }
}

void ClientHandler::acceptNewClients(ClientMap& clientMap)
{
    // If we're at max capacity, reject any waiting connections.
    if (clientCount == Config::MAX_CLIENTS) {
        while (acceptor.reject()) {
            LOG_INFO("Rejected connection attempt: Already at maximum "
                     "connected clients.");
        }
        return;
    }

    // We have room for more peers. Connect to any that are waiting.
    // Note: newPeer adds itself to the socket set.
    std::unique_ptr<Peer> newPeer{acceptor.accept()};
    while (newPeer != nullptr) {
        NetworkID newID{static_cast<NetworkID>(networkIDPool.reserveID())};
        LOG_INFO("New client connected. Assigning netID: %u", newID);

        {
            // Add the peer to the Network's clientMap.
            std::unique_lock writeLock{network.getClientMapMutex()};
            if (!(clientMap
                      .try_emplace(newID, std::make_shared<Client>(
                                              newID, std::move(newPeer)))
                      .second)) {
                networkIDPool.freeID(newID);
                LOG_FATAL(
                    "Ran out of room in client map or key already existed.");
            }
        }

        clientCount++;

        // Notify the sim that a client was connected.
        dispatcher.emplace<ClientConnectionEvent>(ClientConnected{newID});

        newPeer = acceptor.accept();
    }
}

void ClientHandler::eraseDisconnectedClients(ClientMap& clientMap)
{
    /* Erase any disconnected clients. */
    for (auto it = clientMap.begin(); it != clientMap.end();) {
        std::shared_ptr<Client>& client{it->second};

        if (!(client->isConnected())) {
            // Save the ID since we're going to erase this client.
            NetworkID clientID{it->first};

            // Erase the disconnected client.
            networkIDPool.freeID(it->first);
            it = clientMap.erase(it);

            clientCount--;

            // Notify the sim that a client was disconnected.
            LOG_INFO("Erased disconnected client with netID: %u.", clientID);
            dispatcher.emplace<ClientConnectionEvent>(
                ClientDisconnected{clientID});
        }
        else {
            ++it;
        }
    }
}

void ClientHandler::receiveAndProcessClientMessages(ClientMap& clientMap)
{
    // Update each client's internal socket isReady().
    clientSet->checkSockets(0);

    /* Iterate through all clients. */
    // Note: Doesn't need a lock because we only mutate the map from this
    //       thread.
    for (auto& pair : clientMap) {
        const std::shared_ptr<Client>& clientPtr{pair.second};

        // If there's data waiting, try to receive all messages from the
        // client.
        // Note: We can only receive one message at a time, since select()
        //       (checkSockets) only tells us data is available, not how much.
        if (clientPtr->dataIsReady()) {
            Client::ReceiveResult result{clientPtr->receiveMessage()};
            if (result.networkResult == NetworkResult::Success) {
                // Process the message.
                processReceivedMessage(*clientPtr, result.messageType,
                                       result.messageBuffer);
            }
        }
    }
}

void ClientHandler::processReceivedMessage(Client& client, Uint8 messageType,
                                           std::span<Uint8> messageBuffer)
{
    // Handle the message.
    messageProcessor.processReceivedMessage(client.getNetID(), messageType,
                                            messageBuffer.data(),
                                            messageBuffer.size());
}

} // End namespace AccountServer
} // End namespace AM
