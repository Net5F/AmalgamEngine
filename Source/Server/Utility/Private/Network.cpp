#include "Network.h"
#include "Acceptor.h"
#include "Peer.h"
#include <SDL2/SDL_net.h>
#include <algorithm>
#include "Debug.h"

namespace AM
{
namespace Server
{

Network::Network()
: acceptor(nullptr)
, newClientQueue(MAX_QUEUED_NEW_CLIENTS)
, clientSet(std::make_shared<SDLNet_SocketSet>(SDLNet_AllocSocketSet(MAX_CLIENTS)))
, receiveThreadPtr(nullptr)
, exitRequested(false)
, inputQueue(MAX_QUEUED_INPUT_MESSAGES)
{
    SDLNet_Init();

    // We delay creating the acceptor because startup has to be called first.
    acceptor = std::make_unique<Acceptor>(SERVER_PORT, clientSet);

    // Start the receive thread.
    receiveThreadPtr = SDL_CreateThread(Network::pollForMessages, "Receiving Messages",
        (void*) this);
    if (receiveThreadPtr == nullptr) {
        DebugError("Receive thread could not be constructed.");
    }
}

Network::~Network()
{
    SDLNet_Quit();
    exitRequested = true;
    SDL_WaitThread(receiveThreadPtr, NULL);
}

bool Network::send(std::shared_ptr<Peer> client,
                             BinaryBufferSharedPtr message)
{
    if (!(client->isConnected())) {
        DebugInfo("Tried to send while client is disconnected.");
        return false;
    }

    return client->sendMessage(message);
}

bool Network::sendToAll(BinaryBufferSharedPtr message)
{
    bool sendSucceeded = true;

    // Send to all connected clients.
    for (const auto& pair : clients) {
        if (pair.second->isConnected()) {
            if (!send(pair.second, message)) {
                sendSucceeded = false;
            }
        }
    }

    return sendSucceeded;
}

unsigned int Network::getNumInputMessagesWaiting()
{
    return inputQueue.size_approx();
}

BinaryBufferPtr Network::receiveInputMessage()
{
    BinaryBufferPtr message = nullptr;
    if (inputQueue.try_dequeue(message)) {
        return message;
    }
    else {
        return nullptr;
    }
}

void Network::acceptNewClients()
{
    std::shared_ptr<Peer> newClient = acceptor->accept();
    while (newClient != nullptr) {
        DebugInfo("New client connected.");

        if (!(newClientQueue.enqueue(newClient))) {
            DebugError(
                "Ran out of room in new client queue and memory allocation failed.");
        }

        // Creates the new client, which adds itself to the socket set.
        newClient = acceptor->accept();
    }
}

void Network::addClient(EntityID entityID, std::shared_ptr<AM::Peer> client)
{
    clients.emplace(entityID, client);
}

void Network::eraseDisconnectedClients()
{
    for (auto it = clients.begin(); it != clients.end();) {
       if (!(it->second->isConnected())) {
           it = clients.erase(it);
       }
       else {
           ++it;
       }
    }
}

int Network::pollForMessages(void* inNetwork)
{
    Network* network = static_cast<Network*>(inNetwork);

    const std::shared_ptr<SDLNet_SocketSet> clientSet = network->getClientSet();
    const std::unordered_map<EntityID, std::shared_ptr<Peer>>& clients =
        network->getClients();

    std::atomic<bool> const* exitRequested = network->getExitRequestedPtr();

    while (!(*exitRequested)) {
        // Check if there are any new clients to connect.
        network->acceptNewClients();

        // Check if we've detected any disconnected clients.
        network->eraseDisconnectedClients();

        // Check if there's any clients to receive from.
        if (clients.size() == 0) {
            // Delay so we don't waste CPU spinning.
            SDL_Delay(1);
        }
        else {
            receiveClientMessages(network, clientSet, clients);
        }
    }

    return 0;
}

void Network::receiveClientMessages(
Network* network,
const std::shared_ptr<SDLNet_SocketSet> clientSet,
const std::unordered_map<EntityID, std::shared_ptr<Peer>>& clients)
{
    // Check if there are any messages to receive.
    int numReady = SDLNet_CheckSockets(*clientSet, 100);
    if (numReady == -1) {
        DebugInfo("Error while checking sockets: %s", SDLNet_GetError());
        // Most of the time this is a system error, where perror might help.
        perror("SDLNet_CheckSockets");
    }
    else if (numReady > 0) {
        // Receive all messages from all clients.
        for (const auto& pair : clients) {
            BinaryBufferPtr message = pair.second->receiveMessage(false);
            while (message != nullptr) {
                // Queue the message.
                network->queueInputMessage(std::move(message));

                message = pair.second->receiveMessage(false);
            }
        }
    }
}

void Network::queueInputMessage(BinaryBufferPtr message)
{
    if (!(inputQueue.enqueue(std::move(message)))) {
        DebugError("Ran out of room in input queue and memory allocation failed.");
    }
}

std::shared_ptr<Peer> Network::getNewClient()
{
    std::shared_ptr<Peer> client = nullptr;
    if (newClientQueue.try_dequeue(client)) {
        return client;
    }
    else {
        return nullptr;
    }
}

const std::shared_ptr<SDLNet_SocketSet> Network::getClientSet()
{
    return clientSet;
}

const std::unordered_map<EntityID, std::shared_ptr<Peer>>& Network::getClients()
{
    return clients;
}

std::atomic<bool> const* Network::getExitRequestedPtr() {
    return &exitRequested;
}

} // namespace Server
} // namespace AM
