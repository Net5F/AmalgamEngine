#include "Client.h"
#include "Peer.h"
#include "Debug.h"

namespace AM
{
namespace Server
{

Client::Client(std::unique_ptr<Peer> inPeer)
: peer(std::move(inPeer))
{
    // Init the history.
    for (unsigned int i = 0; i < TICKDIFF_HISTORY_LENGTH; ++i) {
        tickDiffHistory.push(0);
    }
}

void Client::queueMessage(const BinaryBufferSharedPtr& message)
{
    sendQueue.push_back(message);
}

NetworkResult Client::sendHeader(const BinaryBufferSharedPtr& header)
{
    if (peer == nullptr) {
        return NetworkResult::Disconnected;
    }

    return peer->send(header);
}

NetworkResult Client::sendWaitingMessages()
{
    if (peer == nullptr) {
        return NetworkResult::Disconnected;
    }

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
    if (peer == nullptr) {
        return {NetworkResult::Disconnected, nullptr};
    }

    return peer->receiveMessage(false);
}

Uint8 Client::getWaitingMessageCount() const
{
    if (peer == nullptr) {
        return 0;
    }

    unsigned int size = sendQueue.size();
    if (size > SDL_MAX_UINT8) {
        DebugError("Client's sendQueue contains too many messages to return as"
        "a Uint8.");
    }

    return size;
}

bool Client::isConnected() {
    // Peer might've been force-disconnected by dropping the reference.
    // It also could have internally detected a client-initiated disconnect.
    return (peer == nullptr) ? false : peer->isConnected();
}

void Client::recordTickDiff(Sint64 tickDiff) {
    if ((tickDiff < LOWEST_VALID_TICKDIFF) || (tickDiff > HIGHEST_VALID_TICKDIFF)) {
        // Diff is outside our bounds. Drop the connection.
        peer = nullptr;
    }
    else {
        // Diff is fine, record it.
        // Acquire a lock so a getTickAdjustment doesn't start while we're pushing.
        std::unique_lock lock(tickDiffMutex);
        tickDiffHistory.push(tickDiff);
    }
}

Sint8 Client::getTickAdjustment() {
    // Acquire a lock so the tickDiffHistory doesn't change while we're processing it.
    std::unique_lock lock(tickDiffMutex);

    // Currently using an average, can make this smarter if necessary.
    int averageDiff = 0;
    for (int i = 0; i < TICKDIFF_HISTORY_LENGTH; ++i) {
        averageDiff += tickDiffHistory[i];
    }
    averageDiff /= TICKDIFF_HISTORY_LENGTH;

    // This cast is guaranteed safe since we check that all incoming values are in range
    // in recordTickDiff.
    return static_cast<Sint8>(averageDiff);
}

} // end namespace Server
} // end namespace AM

