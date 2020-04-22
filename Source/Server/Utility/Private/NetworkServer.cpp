#include "NetworkServer.h"
#include "Acceptor.h"
#include "Peer.h"
#include <SDL2/SDL_net.h>
#include <iostream>

using namespace AM;

const std::string AM::NetworkServer::SERVER_IP = "127.0.0.1";

AM::NetworkServer::NetworkServer()
{
    SDLNet_Init();
    createAcceptor();
}

AM::NetworkServer::~NetworkServer()
{
    SDLNet_Quit();
}

bool AM::NetworkServer::send(std::shared_ptr<Peer> client,
                             BinaryBufferSharedPtr message)
{
    if (!(client->isConnected())) {
        std::cerr << "Tried to send while client is disconnected." << std::endl;
        return false;
    }

    return client->sendMessage(message);
}

AM::BinaryBufferPtr AM::NetworkServer::receive(std::shared_ptr<Peer> client)
{
    if (!(client->isConnected())) {
        std::cerr << "Tried to receive while client is disconnected." << std::endl;
        return nullptr;
    }

    return client->receiveMessage();
}

std::vector<std::shared_ptr<Peer>> AM::NetworkServer::acceptNewClients()
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

void AM::NetworkServer::checkForDisconnections()
{
    for (auto i = clients.begin(); i != clients.end(); ++i) {
        if (!((*i)->isConnected())) {
            clients.erase(i);
        }
    }
}

const std::vector<std::shared_ptr<Peer>>& AM::NetworkServer::getClients()
{
    return clients;
}

void AM::NetworkServer::createAcceptor()
{
    acceptor = std::make_unique<Acceptor>(SERVER_IP, SERVER_PORT);
}
