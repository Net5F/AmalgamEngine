#include <Network.h>
#include "messend.hpp"

const std::string AM::Network::SERVER_IP = "127.0.0.1";

AM::Network::Network()
: acceptor(SERVER_IP, SERVER_PORT)
{
}

AM::Network::~Network()
{
    msnd::shutdown();
}

void AM::Network::send(std::shared_ptr<msnd::Peer> client,
                       const msnd::Message& messageToSend)
{
    if (!(client->isConnected())) {
        std::cerr << "Tried to send while client is disconnected." << std::endl;
        return;
    }

    client->sendMessage(messageToSend);
}

std::unique_ptr<msnd::Message> AM::Network::receive(std::shared_ptr<msnd::Peer> client)
{
    if (!(client->isConnected())) {
        std::cerr << "Tried to receive while client is disconnected." << std::endl;
        return nullptr;
    }

    return client->receiveMessage();
}

std::vector<std::shared_ptr<msnd::Peer>> AM::Network::acceptNewClients()
{
    std::vector<std::shared_ptr<msnd::Peer>> newClients;

    std::shared_ptr<msnd::Peer> newClient = acceptor.accept();
    while (newClient != nullptr) {
        std::cout << "New client connected." << std::endl;
        clients.push_back(newClient);
        newClients.push_back(newClient);

        newClient = acceptor.accept();
    }

    return newClients;
}

void AM::Network::checkForDisconnections()
{
    for (auto i = clients.begin(); i != clients.end(); ++i) {
        if (!((*i)->isConnected())) {
            clients.erase(i);
        }
    }
}

const std::vector<std::shared_ptr<msnd::Peer>>& AM::Network::getClients()
{
    return clients;
}
