#include "Network.h"
#include "Peer.h"
#include <SDL2/SDL_net.h>
#include <iostream>

namespace AM
{
namespace Client
{

const std::string Network::SERVER_IP = "127.0.0.1";

Network::Network()
: server(nullptr)
{
    SDLNet_Init();
}

Network::~Network()
{
    SDLNet_Quit();
}

bool Network::connect()
{
    IPaddress ip;

    // Try to connect.
    server = Peer::initiate(SERVER_IP, SERVER_PORT);
    return (server != nullptr) ? true : false;
}

bool Network::send(BinaryBufferSharedPtr message)
{
    if (!(server->isConnected())) {
        std::cerr << "Tried to send while server is disconnected." << std::endl;
        return false;
    }

    return server->sendMessage(message);
}

BinaryBufferPtr Network::receive()
{
    if (!(server->isConnected())) {
        std::cerr << "Tried to receive while server is disconnected." << std::endl;
        return nullptr;
    }

    return server->receiveMessage();
}

} // namespace Client
} // namespace AM
