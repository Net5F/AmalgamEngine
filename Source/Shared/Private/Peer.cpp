#include "Peer.h"
#include "TcpSocket.h"
#include <SDL_stdinc.h>
#include "Debug.h"

namespace AM
{

std::unique_ptr<Peer> Peer::initiate(std::string serverIP, unsigned int serverPort)
{
    std::unique_ptr<TcpSocket> socket = std::make_unique<TcpSocket>(serverIP, serverPort);

    return std::make_unique<Peer>(std::move(socket));
}

Peer::Peer(std::unique_ptr<TcpSocket> inSocket)
: socket(std::move(inSocket))
, set(std::make_shared<SocketSet>(1)) // No set given, create a set of size 1 for this peer.
, bIsConnected(false)
{
    set->addSocket(*socket);

    bIsConnected = true;
}

Peer::Peer(std::unique_ptr<TcpSocket> inSocket,
           const std::shared_ptr<SocketSet>& inSet)
: socket(std::move(inSocket))
, set(inSet)
, bIsConnected(false)
{
    set->addSocket(*socket);

    bIsConnected = true;
}

Peer::~Peer()
{
    set->remSocket(*socket);
}

bool Peer::isConnected() const
{
    return bIsConnected;
}

NetworkResult Peer::send(const BinaryBufferSharedPtr& message)
{
    if (!bIsConnected) {
        return NetworkResult::Disconnected;
    }

    std::size_t messageSize = message->size();
    if (messageSize > MAX_MESSAGE_SIZE) {
        DebugError("Tried to send a too-large message. Size: %u, max: %u", messageSize,
            MAX_MESSAGE_SIZE);
    }

    int bytesSent = socket->send(message->data(), messageSize);
    if (bytesSent < 0) {
        DebugError("TCP_Send returned < 0. This should never happen, the socket"
                   "was likely misused.");
    }

    if (static_cast<unsigned int>(bytesSent) < messageSize) {
        // The peer probably disconnected (could be a different issue).
        bIsConnected = false;
        return NetworkResult::Disconnected;
    }
    else {
        return NetworkResult::Success;
    }
}

NetworkResult Peer::send(const Uint8* messageBuffer, unsigned int messageSize)
{
    if (!bIsConnected) {
        return NetworkResult::Disconnected;
    }

    if (messageSize > MAX_MESSAGE_SIZE) {
        DebugError("Tried to send a too-large message. Size: %u, max: %u", messageSize,
            MAX_MESSAGE_SIZE);
    }

    int bytesSent = socket->send(messageBuffer, messageSize);
    if (bytesSent < 0) {
        DebugError("TCP_Send returned < 0. This should never happen, the socket"
                   "was likely misused.");
    }

    if (static_cast<unsigned int>(bytesSent) < messageSize) {
        // The peer probably disconnected (could be a different issue).
        bIsConnected = false;
        return NetworkResult::Disconnected;
    }
    else {
        return NetworkResult::Success;
    }
}

ReceiveResult Peer::receiveBytes(Uint16 numBytes, bool checkSockets)
{
    if (!bIsConnected) {
        return {NetworkResult::Disconnected, nullptr};
    }
    else if (checkSockets) {
        // Poll to see if there's data
        set->checkSockets(0);
    }

    if (!(socket->isReady())) {
        return {NetworkResult::NoWaitingData, nullptr};
    }
    else {
        return receiveBytesWait(numBytes);
    }
}

ReceiveResult Peer::receiveBytesWait(Uint16 numBytes)
{
    if (!bIsConnected) {
        return {NetworkResult::Disconnected, nullptr};
    }
    else if (numBytes > MAX_MESSAGE_SIZE) {
        DebugError(
            "Tried to receive too large of a message. messageSize: %u, MaxSize: %u",
            numBytes, MAX_MESSAGE_SIZE);
    }

    BinaryBufferPtr returnBuffer = std::make_unique<BinaryBuffer>(numBytes);
    int result = socket->receive(returnBuffer->data(), numBytes);
    if (result <= 0) {
        // Disconnected
        bIsConnected = false;
        return {NetworkResult::Disconnected, nullptr};
    }
    else if (result < numBytes) {
        DebugError("Didn't receive all the bytes in one chunk."
                   "Need to add logic for this scenario.");
    }

    return {NetworkResult::Success, std::move(returnBuffer)};
}

ReceiveResult Peer::receiveMessage(bool checkSockets)
{
    if (!bIsConnected) {
        return {NetworkResult::Disconnected, nullptr};
    }
    else if (checkSockets) {
        // Poll to see if there's data
        set->checkSockets(0);
    }

    if (!(socket->isReady())) {
        return {NetworkResult::NoWaitingData, nullptr};
    }
    else {
        return receiveMessageWait();
    }
}

ReceiveResult Peer::receiveMessageWait()
{
    if (!bIsConnected) {
        return {NetworkResult::Disconnected, nullptr};
    }

    // Receive the size.
    Uint8 sizeBuf[sizeof(Uint16)];
    int result = socket->receive(sizeBuf, sizeof(Uint16));
    if (result <= 0) {
        // Disconnected
        bIsConnected = false;
        return {NetworkResult::Disconnected, nullptr};
    }
    else if (result < sizeof(Uint16)) {
        DebugError("Didn't receive all size bytes in one chunk."
                   "Need to add logic for this scenario.");
    }

    // The number of bytes in the upcoming message.
    Uint16 messageSize = _SDLNet_Read16(sizeBuf);
    if (messageSize > MAX_MESSAGE_SIZE) {
        DebugError(
            "Tried to receive too large of a message. messageSize: %u, MaxSize: %u",
            messageSize, MAX_MESSAGE_SIZE);
    }

    BinaryBufferPtr returnBuffer = std::make_unique<BinaryBuffer>(messageSize);
    result = socket->receive(returnBuffer->data(), messageSize);
    if (result <= 0) {
        // Disconnected
        bIsConnected = false;
        return {NetworkResult::Disconnected, nullptr};
    }
    else if (result < messageSize) {
        DebugError("Didn't receive all message bytes in one chunk."
                   "Need to add logic for this scenario.");
    }

    return {NetworkResult::Success, std::move(returnBuffer)};
}

} // End namespace AM

