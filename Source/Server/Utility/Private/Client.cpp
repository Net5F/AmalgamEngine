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

NetworkResult Client::sendHeader(BinaryBufferSharedPtr header)
{
    return peer->send(header);
}

void Client::queueMessage(BinaryBufferSharedPtr message)
{
    sendQueue.push_back(message);
}

NetworkResult Client::sendWaitingMessages()
{
    while (!(sendQueue.empty())) {
        BinaryBufferSharedPtr message = sendQueue.front();

        NetworkResult result = peer->send(message);
        if (result != NetworkResult::Success) {
            // Some sort of failure, stop sending and return it.
            return result;
        }

        sendQueue.pop_front();
    }

    return NetworkResult::Success;
}

ReceiveResult Client::receiveMessage()
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

