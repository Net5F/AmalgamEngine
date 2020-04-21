#include <Network.h>
#include "messend.hpp"

const std::string AM::Network::SERVER_IP = "127.0.0.1";

AM::Network::Network()
: server(nullptr)
{
    msnd::startup();
}

AM::Network::~Network()
{
    msnd::shutdown();
}

bool AM::Network::connect()
{
    // Try to connect.
    server = msnd::initiate(SERVER_IP, SERVER_PORT);
    return (server != nullptr) ? true : false;
}

void AM::Network::send(const msnd::Message& messageToSend)
{
    if (!(server->isConnected())) {
        std::cerr << "Tried to send while server is disconnected." << std::endl;
        return;
    }

    server->sendMessage(messageToSend);
}

std::unique_ptr<msnd::Message> AM::Network::receive()
{
    if (!(server->isConnected())) {
        std::cerr << "Tried to receive while server is disconnected." << std::endl;
        return nullptr;
    }

    return server->receiveMessage();
}
