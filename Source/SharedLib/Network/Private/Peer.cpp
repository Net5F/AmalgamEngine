#include "Peer.h"
#include "TcpSocket.h"
#include "ByteTools.h"
#include "Log.h"
#include <SDL_stdinc.h>

namespace AM
{
std::unique_ptr<Peer> Peer::initiate(const std::string& serverIP,
                                     unsigned int serverPort)
{
    TcpSocket socket{};
    if (socket.openConnectionTo(serverIP, static_cast<Uint16>(serverPort))) {
        return std::make_unique<Peer>(std::move(socket));
    }
    else {
        return nullptr;
    }
}

Peer::Peer(TcpSocket&& inSocket)
: socket{std::move(inSocket)}
// No set given, create a set of size 1 for this peer.
, set{std::make_shared<SocketSet>(1)}
, bIsConnected{false}
{
    set->addSocket(socket);

    bIsConnected = true;
}

Peer::Peer(TcpSocket&& inSocket, const std::shared_ptr<SocketSet>& inSet)
: socket{std::move(inSocket)}
, set{inSet}
, bIsConnected{false}
{
    set->addSocket(socket);

    bIsConnected = true;
}

Peer::~Peer()
{
    set->remSocket(socket);
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

    std::size_t messageSize{buffer->size()};
    if (messageSize > MAX_WIRE_SIZE) {
        LOG_FATAL("Tried to send too many bytes. Size: %u, MAX_WIRE_SIZE: %u",
                  messageSize, MAX_WIRE_SIZE);
    }

    int bytesSent{socket.send(buffer->data(), static_cast<int>(messageSize))};
    if (bytesSent < 0) {
        LOG_FATAL("TCP_Send returned < 0. This should never happen, the socket"
                  "was likely misused.");
    }

    if (static_cast<std::size_t>(bytesSent) < messageSize) {
        // The peer probably disconnected (could be a different issue).
        bIsConnected = false;
        return NetworkResult::Disconnected;
    }
    else {
        return NetworkResult::Success;
    }
}

NetworkResult Peer::send(const Uint8* buffer, std::size_t numBytesToSend)
{
    if (!bIsConnected) {
        return NetworkResult::Disconnected;
    }

    if (numBytesToSend > MAX_WIRE_SIZE) {
        LOG_FATAL("Tried to send too many bytes. Size: %u, MAX_WIRE_SIZE: %u",
                  numBytesToSend, MAX_WIRE_SIZE);
    }

    int bytesSent{socket.send(buffer, static_cast<int>(numBytesToSend))};
    if (bytesSent < 0) {
        LOG_FATAL("TCP_Send returned < 0. This should never happen, the socket"
                  "was likely misused.");
    }

    if (static_cast<std::size_t>(bytesSent) < numBytesToSend) {
        // The peer probably disconnected (could be a different issue).
        bIsConnected = false;
        return NetworkResult::Disconnected;
    }
    else {
        return NetworkResult::Success;
    }
}

NetworkResult Peer::receiveBytes(Uint8* buffer, std::size_t numBytes,
                                 bool checkSockets)
{
    if (!bIsConnected) {
        return NetworkResult::Disconnected;
    }
    else if (checkSockets) {
        // Poll to see if there's data
        set->checkSockets(0);
    }

    if (!(socket.isReady())) {
        return NetworkResult::NoWaitingData;
    }
    else {
        return receiveBytesWait(buffer, numBytes);
    }
}

NetworkResult Peer::receiveBytesWait(Uint8* buffer, std::size_t numBytes)
{
    if (!bIsConnected) {
        return NetworkResult::Disconnected;
    }

    // Loop until we've received all of the bytes.
    int bytesReceived{0};
    while (static_cast<std::size_t>(bytesReceived) < numBytes) {
        // Try to receive bytes.
        int result{socket.receive((buffer + bytesReceived),
                                  static_cast<int>(numBytes))};
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

    if (!(socket.isReady())) {
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
    int result{socket.receive(headerBuf, MESSAGE_HEADER_SIZE)};
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

    result = socket.receive(messageBuffer, messageSize);
    if (result <= 0) {
        // Disconnected
        bIsConnected = false;
        return {NetworkResult::Disconnected};
    }
    else if (result < messageSize) {
        LOG_FATAL("Didn't receive all message bytes in one chunk."
                  "Need to add logic for this scenario.");
    }

    Uint8 messageType{headerBuf[MessageHeaderIndex::MessageType]};
    return {NetworkResult::Success, messageType, messageSize};
}

ReceiveResult Peer::receiveMessageWait(BinaryBufferPtr& messageBuffer)
{
    if (!bIsConnected) {
        return {NetworkResult::Disconnected};
    }

    // Receive the message header.
    std::array<Uint8, MESSAGE_HEADER_SIZE> headerBuf{};
    int result{socket.receive(headerBuf.data(), MESSAGE_HEADER_SIZE)};
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

    messageBuffer = std::make_unique<BinaryBuffer>(messageSize);
    result = socket.receive(messageBuffer->data(), messageSize);
    if (result <= 0) {
        // Disconnected
        bIsConnected = false;
        return {NetworkResult::Disconnected};
    }
    else if (result < messageSize) {
        LOG_FATAL("Didn't receive all message bytes in one chunk."
                  "Need to add logic for this scenario.");
    }

    Uint8 messageType{headerBuf[MessageHeaderIndex::MessageType]};
    return {NetworkResult::Success, messageType, messageSize};
}

} // End namespace AM
