#include "Client.h"
#include "Peer.h"
#include "Debug.h"

namespace AM
{
namespace Server
{

Client::Client(std::shared_ptr<Peer> inPeer)
: peer(inPeer)
{
}

bool Client::isConnected() const
{
    return peer->isConnected();
}

bool Client::send(BinaryBufferSharedPtr message)
{
    return peer->send(message);
}

bool Client::sendWaitingMessages()
{
    BinaryBufferSharedPtr message = sendQueue.front();
    while (message != nullptr) {
        if (!(peer->send(message))) {
            return false;
        }

        sendQueue.pop_front();
        message = sendQueue.front();
    }

    return true;
}

BinaryBufferSharedPtr Client::receiveMessage()
{
    return peer->receiveMessage(false);
}

Uint8 Client::getWaitingMessageCount() const
{
    unsigned int size = sendQueue.size();
    if (size > SDL_MAX_UINT8) {
        DebugError("Client's sendQueue contains too many messages to return as"
        "a Uint8.");
    }

    return size;
}

} // end namespace Server
} // end namespace AM

