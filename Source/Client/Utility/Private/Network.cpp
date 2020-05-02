#include "Network.h"
#include "Peer.h"
#include <SDL2/SDL_net.h>
#include "Debug.h"

namespace AM
{
namespace Client
{

const std::string Network::SERVER_IP = "127.0.0.1";

Network::Network()
: server(nullptr)
, receiveThreadPtr(nullptr)
, exitRequested(false)
{
    SDLNet_Init();
}

Network::~Network()
{
    SDLNet_Quit();
    exitRequested = true;
    SDL_WaitThread(receiveThreadPtr, NULL);
}

bool Network::connect()
{
    IPaddress ip;

    // Try to connect.
    server = Peer::initiate(SERVER_IP, SERVER_PORT);

    // Spin up the receive thread.
    if (server != nullptr) {
        receiveThreadPtr = SDL_CreateThread(Network::pollForMessages, "Receiving Messages",
            (void*) this);
        if (receiveThreadPtr == nullptr) {
            DebugError("Receive thread could not be constructed.");
        }
    }

    return (server != nullptr) ? true : false;
}

bool Network::send(BinaryBufferSharedPtr message)
{
    if (!(server->isConnected())) {
        DebugInfo("Tried to send while server is disconnected.");
        return false;
    }

    return server->sendMessage(message);
}

BinaryBufferPtr Network::receive()
{
    if (!(server->isConnected())) {
        DebugInfo("Tried to receive while server is disconnected.");
        return nullptr;
    }

    BinaryBufferPtr message = nullptr;
    if (receiveQueue.try_dequeue(message)) {
        return message;
    }
    else {
        return nullptr;
    }
}

int Network::pollForMessages(void* inNetwork)
{
    Network* network = static_cast<Network*>(inNetwork);
    std::shared_ptr<Peer> server = network->getServer();
    std::atomic<bool> const* exitRequested = network->getExitRequestedPtr();

    while (!(*exitRequested)) {
        // Check if there are any messages to receive.
        BinaryBufferPtr message = server->receiveMessageWait();

        // If we received a message, push it into the queue.
        if (message != nullptr) {
            network->queueMessage(std::move(message));
        }
    }

    return 0;
}

void Network::queueMessage(BinaryBufferPtr message)
{
    if (!(receiveQueue.enqueue(std::move(message)))) {
        DebugError("Ran out of room in receive queue and memory allocation failed.");
    }
}

std::shared_ptr<Peer> Network::getServer()
{
    return server;
}

std::atomic<bool> const* Network::getExitRequestedPtr() {
    return &exitRequested;
}

} // namespace Client
} // namespace AM
