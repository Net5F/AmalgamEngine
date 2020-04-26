#include "Network.h"
#include "Acceptor.h"
#include "Peer.h"
#include <SDL2/SDL_net.h>
#include <algorithm>
#include <iostream>

namespace AM
{
namespace Server
{

Network::Network()
: acceptor(nullptr)
{
    SDLNet_Init();

    // We delay creating the acceptor because startup has to be called first.
    acceptor = std::make_unique<Acceptor>(SERVER_PORT);
}

Network::~Network()
{
    SDLNet_Quit();
}

bool Network::send(std::shared_ptr<Peer> client,
                             BinaryBufferSharedPtr message)
{
    if (!(client->isConnected())) {
        std::cerr << "Tried to send while client is disconnected." << std::endl;
        return false;
    }

    return client->sendMessage(message);
}

bool Network::sendToAll(BinaryBufferSharedPtr message)
{
    bool sendSucceeded = true;

    // Send to all connected clients.
    for (std::shared_ptr<Peer> client : clients) {
        if (client->isConnected()) {
            if (!send(client, message)) {
                sendSucceeded = false;
            }
        }
    }

    return sendSucceeded;
}

BinaryBufferPtr Network::receive(std::shared_ptr<Peer> client)
{
    if (!(client->isConnected())) {
        std::cerr << "Tried to receive while client is disconnected." << std::endl;
        return nullptr;
    }

    return client->receiveMessage();
}

std::vector<std::shared_ptr<Peer>> Network::acceptNewClients()
{
    std::vector<std::shared_ptr<Peer>> newClients;

    std::shared_ptr<Peer> newClient = acceptor->accept();
    while (newClient != nullptr) {
        std::cout << "New client connected." << std::endl;
        clients.push_back(newClient);
        newClients.push_back(newClient);

        newClient = acceptor->accept();
    }

    return newClients;
}

void Network::checkForDisconnections()
{
    clients.erase(std::remove_if(clients.begin(), clients.end(), IsDisconnected),
        clients.end());
}

const std::vector<std::shared_ptr<Peer>>& Network::getClients()
{
    return clients;
}

bool Network::IsDisconnected(const std::shared_ptr<Peer>& peer)
{
    return !(peer->isConnected());
}

} // namespace Server
} // namespace AM
