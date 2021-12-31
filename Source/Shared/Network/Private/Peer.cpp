#include "Peer.h"
#include "TcpSocket.h"
#include "ByteTools.h"
#include "Log.h"
#include <SDL2/SDL_stdinc.h>

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

NetworkResult Peer::send(const BinaryBufferSharedPtr& buffer)
{
    if (!bIsConnected) {
        return NetworkResult::Disconnected;
    }

    std::size_t messageSize = buffer->size();
    if (messageSize > MAX_WIRE_SIZE) {
        LOG_FATAL("Tried to send too many bytes. Size: %u, MAX_WIRE_SIZE: %u",
                  messageSize, MAX_WIRE_SIZE);
    }

    int bytesSent = socket->send(buffer->data(), messageSize);
    if (bytesSent < 0) {
        LOG_FATAL("TCP_Send returned < 0. This should never happen, the socket"
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

NetworkResult Peer::send(const Uint8* buffer, unsigned int numBytesToSend)
{
    if (!bIsConnected) {
        return NetworkResult::Disconnected;
    }

    if (numBytesToSend > MAX_WIRE_SIZE) {
        LOG_FATAL("Tried to send too many bytes. Size: %u, MAX_WIRE_SIZE: %u",
                  numBytesToSend, MAX_WIRE_SIZE);
    }

    int bytesSent{socket->send(buffer, numBytesToSend)};
    if (bytesSent < 0) {
        LOG_FATAL("TCP_Send returned < 0. This should never happen, the socket"
                  "was likely misused.");
    }

    if (static_cast<unsigned int>(bytesSent) < numBytesToSend) {
        // The peer probably disconnected (could be a different issue).
        bIsConnected = false;
        return NetworkResult::Disconnected;
    }
    else {
        return NetworkResult::Success;
    }
}

NetworkResult Peer::receiveBytes(Uint8* buffer, unsigned int numBytes,
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
        return receiveBytesWait(buffer, numBytes);
    }
}

NetworkResult Peer::receiveBytesWait(Uint8* buffer, unsigned int numBytes)
{
    if (!bIsConnected) {
        return NetworkResult::Disconnected;
    }

    // Loop until we've received all of the bytes.
    int bytesReceived{0};
    while (static_cast<unsigned int>(bytesReceived) < numBytes) {
        // Try to receive bytes.
        int result{socket->receive((buffer + bytesReceived), numBytes)};
        if (result > 0) {
            bytesReceived += result;
        }
        else {
            // Disconnected
            bIsConnected = false;
            return NetworkResult::Disconnected;
        }
    }

    return NetworkResult::Success;
}

ReceiveResult Peer::receiveMessage(Uint8* messageBuffer, bool checkSockets)
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

ReceiveResult Peer::receiveMessageWait(Uint8* messageBuffer)
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
        LOG_FATAL("Didn't receive all size bytes in one chunk."
                  "Need to add logic for this scenario.");
    }

    // The number of bytes in the upcoming message.
    Uint16 messageSize{
        ByteTools::read16(&(headerBuf[MessageHeaderIndex::Size]))};
    if (messageSize > MAX_WIRE_SIZE) {
        LOG_FATAL("Tried to receive too large of a message. messageSize: %u, "
                  "MAX_WIRE_SIZE: %u",
                  messageSize, MAX_WIRE_SIZE);
    }

    result = socket->receive(messageBuffer, messageSize);
    if (result <= 0) {
        // Disconnected
        bIsConnected = false;
        return {NetworkResult::Disconnected};
    }
    else if (result < messageSize) {
        LOG_FATAL("Didn't receive all message bytes in one chunk."
                  "Need to add logic for this scenario.");
    }

    MessageType messageType
        = static_cast<MessageType>(headerBuf[MessageHeaderIndex::MessageType]);
    return {NetworkResult::Success, messageType, messageSize};
}

ReceiveResult Peer::receiveMessageWait(BinaryBufferPtr& messageBuffer)
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
        LOG_FATAL("Didn't receive all size bytes in one chunk."
                  "Need to add logic for this scenario.");
    }

    // The number of bytes in the upcoming message.
    Uint16 messageSize
        = ByteTools::read16(&(headerBuf[MessageHeaderIndex::Size]));
    if (messageSize > MAX_WIRE_SIZE) {
        LOG_FATAL("Tried to receive too large of a message. messageSize: %u, "
                  "MAX_WIRE_SIZE: %u",
                  messageSize, MAX_WIRE_SIZE);
    }

    messageBuffer = std::make_unique<BinaryBuffer>(messageSize);
    result = socket->receive(messageBuffer->data(), messageSize);
    if (result <= 0) {
        // Disconnected
        bIsConnected = false;
        return {NetworkResult::Disconnected};
    }
    else if (result < messageSize) {
        LOG_FATAL("Didn't receive all message bytes in one chunk."
                  "Need to add logic for this scenario.");
    }

    MessageType messageType
        = static_cast<MessageType>(headerBuf[MessageHeaderIndex::MessageType]);
    return {NetworkResult::Success, messageType, messageSize};
}

} // End namespace AM
