#include "ClientHandler.h"
#include "WorldClientEndpoint.h"
#include "AccountServiceEndpoint.h"
#include "NetworkDefs.h"
#include "SocketSet.h"
#include "ClientConnectionEvent.h"
#include "ConnectionRequest.h"
#include "Deserialize.h"
#include "Config.h"
#include "Log.h"
#include <shared_mutex>
#include <mutex>
#include <memory>

namespace AM
{
namespace WorldServer
{

ClientHandler::ClientHandler(WorldClientEndpoint& inEndpoint,
                             EventDispatcher& inDispatcher,
                             MessageProcessor& inMessageProcessor,
                             AccountServiceEndpoint& inAccountEndpoint)
: endpoint{inEndpoint}
, dispatcher{inDispatcher}
, messageProcessor{inMessageProcessor}
, accountEndpoint{inAccountEndpoint}
, ticketResultQueue{}
, networkIDPool{IDPool::ReservationStrategy::MarchForward, 8}
, clientCount{0}
, clientSet{std::make_shared<SocketSet>(Config::MAX_CLIENTS)}
, acceptor{Config::SERVER_PORT, clientSet}
, receiveThreadObj{}
, exitRequested{false}
, sendRequested{false}
{
    // Start the send and receive threads.
    receiveThreadObj = std::thread(&ClientHandler::serviceClients, this);
    sendThreadObj = std::thread(&ClientHandler::sendClientUpdates, this);

    // Reserve the null network ID.
    networkIDPool.reserveID();
}

ClientHandler::~ClientHandler()
{
    exitRequested = true;
    receiveThreadObj.join();

    {
        std::unique_lock lock{sendMutex};
        sendRequested = true;
    }
    sendCondVar.notify_one();
    sendThreadObj.join();
}

void ClientHandler::beginSendClientUpdates()
{
    // Wake the send thread.
    {
        std::unique_lock lock{sendMutex};
        sendRequested = true;
    }
    sendCondVar.notify_one();
}

void ClientHandler::serviceClients()
{
    tracy::SetThreadName("ServerReceive");

    ClientMap& clientMap{endpoint.getClientMap()};

    while (!exitRequested) {
        // Check if there are any new clients to connect.
        acceptNewClients(clientMap);

        // Authenticate or reject any clients whose tickets were validated.
        processTicketResults(clientMap);

        // Erase any clients who were detected to be disconnected.
        eraseDisconnectedClients(clientMap);

        // Check if there's any clients with activity, and process all their
        // messages.
        // Note: Doesn't need a lock because we only mutate the map from this
        //       thread.
        int numReceived = 0;
        if (clientMap.size() != 0) {
            numReceived = receiveAndProcessClientMessages(clientMap);
        }

        // There wasn't any activity, delay so we don't waste CPU spinning.
        if (numReceived == 0) {
            SDL_Delay(INACTIVE_DELAY_TIME_MS);
        }
    }
}

void ClientHandler::sendClientUpdates()
{
    tracy::SetThreadName("ServerSend");

    SharedLockableBase(std::shared_mutex)
        & clientMapMutex{endpoint.getClientMapMutex()};
    ClientMap& clientMap{endpoint.getClientMap()};

    while (!exitRequested) {
        // Wait until this thread is signaled by beginSendClientUpdates().
        std::unique_lock lock{sendMutex};
        sendCondVar.wait(lock, [this] { return sendRequested; });

        {
            ZoneScoped;

            // Acquire a read lock before running through the client map.
            std::shared_lock readLock{clientMapMutex};

            // Run through the clients, sending their waiting messages.
            Uint32 currentTick{endpoint.getCurrentTick()};
            for (auto& pair : clientMap) {
                pair.second->sendWaitingMessages(currentTick);
            }

            sendRequested = false;
        }
    }
}

void ClientHandler::acceptNewClients(ClientMap& clientMap)
{
    ZoneScoped;

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
            // Add the peer to the endpoint's client map.
            std::unique_lock writeLock{endpoint.getClientMapMutex()};
            if (!(clientMap
                      .try_emplace(newID, std::make_shared<Client>(
                                              newID, std::move(newPeer)))
                      .second)) {
                LOG_ERROR(
                    "Ran out of room in client map or key already existed.");
                networkIDPool.freeID(newID);
                continue;
            }
        }

        clientCount++;

        // Note: We don't notify the sim about this client until it
        //       authenticates (see processTicketResults()).

        newPeer = acceptor.accept();
    }
}

void ClientHandler::processTicketResults(ClientMap& clientMap)
{
    ZoneScoped;

    TicketResult ticketResult{};
    while (ticketResultQueue.try_dequeue(ticketResult)) {
        // If the client disconnected while we were waiting, there's nothing
        // to do.
        // Note: We compare the pointers in case the netID was reused.
        std::shared_ptr<Client> client{ticketResult.client.lock()};
        auto clientIt{clientMap.find(ticketResult.netID)};
        if (!client || (clientIt == clientMap.end())
            || (clientIt->second != client)) {
            continue;
        }

        if (client->getAuthState() != Client::AuthState::Validating) {
            LOG_ERROR("Received ticket result for client that isn't "
                      "validating. NetID: %u",
                      ticketResult.netID);
            continue;
        }

        const ConsumeWorldTicketResponse& response{ticketResult.response};
        switch (response.result) {
            case ConsumeWorldTicketResponse::Success: {
                client->setAuthState(Client::AuthState::Authenticated);
                LOG_INFO("Client authenticated. NetID: %u, AccountID: %lld",
                         ticketResult.netID,
                         static_cast<long long>(response.accountID));

                // Notify the sim that a client was connected.
                dispatcher.emplace<ClientConnectionEvent>(
                    ClientConnected{ticketResult.netID, response.accountID});
                break;
            }
            case ConsumeWorldTicketResponse::InvalidTicket: {
                LOG_INFO("Rejected client: Invalid ticket. NetID: %u",
                         ticketResult.netID);
                rejectClient(*client, ConnectionResponse::InvalidTicket);
                break;
            }
            default: {
                LOG_INFO("Rejected client: Failed to validate ticket. "
                         "NetID: %u",
                         ticketResult.netID);
                rejectClient(*client, ConnectionResponse::InternalError);
                break;
            }
        }
    }
}

void ClientHandler::eraseDisconnectedClients(ClientMap& clientMap)
{
    ZoneScoped;

    /* Erase any disconnected clients. */
    for (auto it = clientMap.begin(); it != clientMap.end();) {
        std::shared_ptr<Client>& client{it->second};

        if (!(client->isConnected())) {
            // Save the ID and auth state since we're going to erase this
            // client.
            NetworkID clientID{it->first};
            bool wasAuthenticated{client->getAuthState()
                                  == Client::AuthState::Authenticated};

            {
                // Need to modify the map, acquire a write lock.
                std::unique_lock writeLock{endpoint.getClientMapMutex()};

                // Erase the disconnected client.
                networkIDPool.freeID(it->first);
                it = clientMap.erase(it);
            }

            clientCount--;

            // If the sim knows about this client, notify it that the client
            // was disconnected.
            LOG_INFO("Erased disconnected client with netID: %u.", clientID);
            if (wasAuthenticated) {
                dispatcher.emplace<ClientConnectionEvent>(
                    ClientDisconnected{clientID});
            }
        }
        else {
            ++it;
        }
    }
}

int ClientHandler::receiveAndProcessClientMessages(ClientMap& clientMap)
{
    ZoneScoped;

    // Update each client's internal socket isReady().
    clientSet->checkSockets(0);

    /* Iterate through all clients. */
    // Note: Doesn't need a lock because we only mutate the map from this
    //       thread.
    int numReceived{0};
    for (auto& pair : clientMap) {
        const std::shared_ptr<Client>& clientPtr{pair.second};

        // If there's data waiting, try to receive all messages from the
        // client.
        // Note: We can only receive one message at a time, since select()
        //       (checkSockets) only tells us data is available, not how much.
        if (clientPtr->dataIsReady()) {
            Client::ReceiveResult result{clientPtr->receiveMessage()};
            if (result.networkResult == NetworkResult::Success) {
                numReceived++;

                // Process the message.
                // Note: Until a client authenticates, the only message we
                //       accept from it is its ConnectionRequest.
                if (result.messageType
                    == static_cast<Uint8>(
                        EngineMessageType::ConnectionRequest)) {
                    processConnectionRequest(clientPtr, result.messageBuffer);
                }
                else if (clientPtr->getAuthState()
                         == Client::AuthState::Authenticated) {
                    processReceivedMessage(*clientPtr, result.messageType,
                                           result.messageBuffer);
                }
            }
        }
    }

    return numReceived;
}

void ClientHandler::processReceivedMessage(Client& client, Uint8 messageType,
                                           std::span<Uint8> messageBuffer)
{
    // Process the message.
    // Note: messageTick will be > -1 if the message contained a tick number.
    Sint64 messageTick{messageProcessor.processReceivedMessage(
        client.getNetID(), messageType, messageBuffer.data(),
        messageBuffer.size())};

    // If the message carried a tick number, use it to calc a diff and give it
    // to the client.
    if (messageTick != -1) {
        // Calc the difference between the current tick and the message's tick.
        Sint64 tickDiff{messageTick
                        - static_cast<Sint64>(endpoint.getCurrentTick())};

        // Record the diff.
        client.recordTickDiff(tickDiff);
    }
}

void ClientHandler::processConnectionRequest(
    const std::shared_ptr<Client>& client, std::span<Uint8> messageBuffer)
{
    NetworkID netID{client->getNetID()};
    if (client->getAuthState() != Client::AuthState::AwaitingRequest) {
        LOG_INFO("Ignoring unexpected ConnectionRequest. NetID: %u", netID);
        return;
    }

    ConnectionRequest connectionRequest{};
    if (!Deserialize::fromBuffer(messageBuffer.data(), messageBuffer.size(),
                                 connectionRequest)) {
        LOG_INFO("Rejected client: Failed to deserialize ConnectionRequest. "
                 "NetID: %u",
                 netID);
        rejectClient(*client, ConnectionResponse::InvalidTicket);
        return;
    }

    // Ask the AccountServer to validate the ticket.
    // Note: The callback is called on the network IO thread, so we pass the
    //       result to processTicketResults() through a queue.
    client->setAuthState(Client::AuthState::Validating);
    std::weak_ptr<Client> weakClient{client};
    accountEndpoint.consumeWorldTicket(
        connectionRequest.ticket,
        [this, netID, weakClient](const ConsumeWorldTicketResponse& response) {
            ticketResultQueue.enqueue(
                TicketResult{netID, weakClient, response});
        });
}

void ClientHandler::rejectClient(Client& client,
                                 ConnectionResponse::Result result)
{
    client.setAuthState(Client::AuthState::Rejected);
    client.queueMessage(endpoint.serialize(ConnectionResponse{.result{result}}),
                        0);
}

} // End namespace WorldServer
} // End namespace AM
