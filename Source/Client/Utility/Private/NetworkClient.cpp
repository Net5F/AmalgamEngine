#include "NetworkClient.h"
#include "Peer.h"
#include <SDL2/SDL_net.h>
#include <iostream>

using namespace AM;

const std::string AM::NetworkClient::SERVER_IP = "127.0.0.1";

AM::NetworkClient::NetworkClient()
: server(nullptr)
{
    SDLNet_Init();
}

AM::NetworkClient::~NetworkClient()
{
    SDLNet_Quit();
}

bool AM::NetworkClient::connect()
{
    IPaddress ip;

    // Try to connect.
    server = Peer::initiate(SERVER_IP, SERVER_PORT);
    return (server != nullptr) ? true : false;
}

bool AM::NetworkClient::send(BinaryBufferSharedPtr message)
{
    if (!(server->isConnected())) {
        std::cerr << "Tried to send while server is disconnected." << std::endl;
        return false;
    }

    return server->sendMessage(message);
}

BinaryBufferPtr AM::NetworkClient::receive()
{
    if (!(server->isConnected())) {
        std::cerr << "Tried to receive while server is disconnected." << std::endl;
        return nullptr;
    }

    return server->receiveMessage();
}
