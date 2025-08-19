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

bool Peer::isReady(bool checkSockets)
{
    if (checkSockets) {
        // Poll to see if there's data
        set->checkSockets(0);
    }

    return socket.isReady();
}

int Peer::receiveBytes(Uint8* buffer, std::size_t numBytes)
{
    if (!bIsConnected) {
        return -1;
    }

    // Try to receive bytes.
    int bytesReceived{socket.receive(buffer, static_cast<int>(numBytes))};
    if (bytesReceived < 0) {
        // Disconnected
        bIsConnected = false;
        return -1;
    }

    return bytesReceived;
}

int Peer::receiveBytesWait(Uint8* buffer, std::size_t numBytes)
{
    if (!bIsConnected) {
        return -1;
    }

    // Loop until we've received all of the bytes.
    int bytesReceived{0};
    while (static_cast<std::size_t>(bytesReceived) < numBytes) {
        // Try to receive bytes.
        int result{
            socket.receive((buffer + bytesReceived),
                           (static_cast<int>(numBytes) - bytesReceived))};
        if (result > 0) {
            bytesReceived += result;
        }
        else {
            // Disconnected
            bIsConnected = false;
            return -1;
        }
    }

    return bytesReceived;
}

} // End namespace AM
