#include "Peer.h"
#include "TcpSocket.h"
#include <SDL_stdinc.h>
#include "Log.h"

namespace AM
{
std::unique_ptr<Peer> Peer::initiate(std::string serverIP,
                                     unsigned int serverPort)
{
    std::unique_ptr<TcpSocket> socket
        = std::make_unique<TcpSocket>(serverIP, serverPort);

    return std::make_unique<Peer>(std::move(socket));
}

Peer::Peer(std::unique_ptr<TcpSocket> inSocket)
: socket(std::move(inSocket))
, set(std::make_shared<SocketSet>(
      1)) // No set given, create a set of size 1 for this peer.
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
        LOG_ERROR("Tried to send a too-large message. Size: %u, max: %u",
                  messageSize, MAX_MESSAGE_SIZE);
    }

    int bytesSent = socket->send(message->data(), messageSize);
    if (bytesSent < 0) {
        LOG_ERROR("TCP_Send returned < 0. This should never happen, the socket"
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
        LOG_ERROR("Tried to send a too-large message. Size: %u, max: %u",
                  messageSize, MAX_MESSAGE_SIZE);
    }

    int bytesSent = socket->send(messageBuffer, messageSize);
    if (bytesSent < 0) {
        LOG_ERROR("TCP_Send returned < 0. This should never happen, the socket"
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

NetworkResult Peer::receiveBytes(Uint8* messageBuffer, Uint16 numBytes,
                                 bool checkSockets)
{
    if (!bIsConnected) {
        return NetworkResult::Disconnected;
    }
    else if (checkSockets) {
        // Poll to see if there's data
        set->checkSockets(0);
    }

    if (!(socket->isReady())) {
        return NetworkResult::NoWaitingData;
    }
    else {
        return receiveBytesWait(messageBuffer, numBytes);
    }
}

NetworkResult Peer::receiveBytesWait(Uint8* messageBuffer, Uint16 numBytes)
{
    if (!bIsConnected) {
        return NetworkResult::Disconnected;
    }
    else if (numBytes > MAX_MESSAGE_SIZE) {
        LOG_ERROR("Tried to receive too large of a message. messageSize: %u, "
                  "MaxSize: %u",
                  numBytes, MAX_MESSAGE_SIZE);
    }

    int result = socket->receive(messageBuffer, numBytes);
    if (result <= 0) {
        // Disconnected
        bIsConnected = false;
        return NetworkResult::Disconnected;
    }
    else if (result < numBytes) {
        LOG_ERROR("Didn't receive all the bytes in one chunk."
                  "Need to add logic for this scenario.");
    }

    return NetworkResult::Success;
}

MessageResult Peer::receiveMessage(Uint8* messageBuffer, bool checkSockets)
{
    if (!bIsConnected) {
        return {NetworkResult::Disconnected};
    }
    else if (checkSockets) {
        // Poll to see if there's data
        set->checkSockets(0);
    }

    if (!(socket->isReady())) {
        return {NetworkResult::NoWaitingData};
    }
    else {
        return receiveMessageWait(messageBuffer);
    }
}

MessageResult Peer::receiveMessageWait(Uint8* messageBuffer)
{
    if (!bIsConnected) {
        return {NetworkResult::Disconnected};
    }

    // Receive the message header.
    Uint8 headerBuf[MESSAGE_HEADER_SIZE];
    int result = socket->receive(headerBuf, MESSAGE_HEADER_SIZE);
    if (result <= 0) {
        // Disconnected
        bIsConnected = false;
        return {NetworkResult::Disconnected};
    }
    else if (result < static_cast<int>(MESSAGE_HEADER_SIZE)) {
        LOG_ERROR("Didn't receive all size bytes in one chunk."
                  "Need to add logic for this scenario.");
    }

    // The number of bytes in the upcoming message.
    Uint16 messageSize = _SDLNet_Read16(&(headerBuf[MessageHeaderIndex::Size]));
    if (messageSize > MAX_MESSAGE_SIZE) {
        LOG_ERROR("Tried to receive too large of a message. messageSize: %u, "
                  "MaxSize: %u",
                  messageSize, MAX_MESSAGE_SIZE);
    }

    result = socket->receive(messageBuffer, messageSize);
    if (result <= 0) {
        // Disconnected
        bIsConnected = false;
        return {NetworkResult::Disconnected};
    }
    else if (result < messageSize) {
        LOG_ERROR("Didn't receive all message bytes in one chunk."
                  "Need to add logic for this scenario.");
    }

    MessageType messageType
        = static_cast<MessageType>(headerBuf[MessageHeaderIndex::MessageType]);
    return {NetworkResult::Success, messageType, messageSize};
}

MessageResult Peer::receiveMessageWait(BinaryBufferPtr& messageBuffer)
{
    if (!bIsConnected) {
        return {NetworkResult::Disconnected};
    }

    // Receive the message header.
    Uint8 headerBuf[MESSAGE_HEADER_SIZE];
    int result = socket->receive(headerBuf, MESSAGE_HEADER_SIZE);
    if (result <= 0) {
        // Disconnected
        bIsConnected = false;
        return {NetworkResult::Disconnected};
    }
    else if (result < static_cast<int>(MESSAGE_HEADER_SIZE)) {
        LOG_ERROR("Didn't receive all size bytes in one chunk."
                  "Need to add logic for this scenario.");
    }

    // The number of bytes in the upcoming message.
    Uint16 messageSize = _SDLNet_Read16(&(headerBuf[MessageHeaderIndex::Size]));
    if (messageSize > MAX_MESSAGE_SIZE) {
        LOG_ERROR("Tried to receive too large of a message. messageSize: %u, "
                  "MaxSize: %u",
                  messageSize, MAX_MESSAGE_SIZE);
    }

    messageBuffer = std::make_unique<BinaryBuffer>(messageSize);
    result = socket->receive(messageBuffer->data(), messageSize);
    if (result <= 0) {
        // Disconnected
        bIsConnected = false;
        return {NetworkResult::Disconnected};
    }
    else if (result < messageSize) {
        LOG_ERROR("Didn't receive all message bytes in one chunk."
                  "Need to add logic for this scenario.");
    }

    MessageType messageType
        = static_cast<MessageType>(headerBuf[MessageHeaderIndex::MessageType]);
    return {NetworkResult::Success, messageType, messageSize};
}

} // End namespace AM
