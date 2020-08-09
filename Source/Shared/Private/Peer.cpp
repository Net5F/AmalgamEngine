#include "Peer.h"
#include <SDL_stdinc.h>
#include "Debug.h"
#include "Message_generated.h"

using namespace AM;

std::unique_ptr<Peer> AM::Peer::initiate(std::string serverIP,
                                         unsigned int serverPort)
{
    IPaddress ip;

    if (SDLNet_ResolveHost(&ip, serverIP.c_str(), serverPort)) {
        DebugInfo("Could not resolve host");
    }

    TCPsocket socket = SDLNet_TCP_Open(&ip);
    if (!socket) {
        DebugInfo("Could not open socket");
    }

    return std::make_unique<Peer>(socket);
}

AM::Peer::Peer(TCPsocket inSocket)
: set(nullptr)
, bIsConnected(false)
{
    set = std::make_shared<SDLNet_SocketSet>(SDLNet_AllocSocketSet(1));
    if (!(*set)) {
        DebugInfo("Error allocating socket set: %s", SDLNet_GetError());
    }

    int numAdded = SDLNet_TCP_AddSocket(*set, inSocket);
    if (numAdded < 1) {
        DebugInfo("Error while adding socket: %s", SDLNet_GetError());
    }

    socket = inSocket;

    bIsConnected = true;
}

AM::Peer::Peer(TCPsocket inSocket, const std::shared_ptr<SDLNet_SocketSet>& inSet)
: set(inSet)
, bIsConnected(false)
{
    if (!(*set)) {
        DebugInfo("Tried to init Peer with bad set.");
    }

    int numAdded = SDLNet_TCP_AddSocket(*set, inSocket);
    if (numAdded < 1) {
        DebugInfo("Error while adding socket: %s", SDLNet_GetError());
    }

    socket = inSocket;

    bIsConnected = true;
}

AM::Peer::~Peer()
{
    SDLNet_TCP_DelSocket(*set, socket);

    // If we're the only one using this set, free it.
    if (set.use_count() == 1) {
        SDLNet_FreeSocketSet(*set);
    }

    SDLNet_TCP_Close(socket);
}

bool AM::Peer::isConnected() const
{
    return bIsConnected;
}

AM::NetworkResult AM::Peer::send(const BinaryBufferSharedPtr& message)
{
    if (!bIsConnected) {
        return NetworkResult::Disconnected;
    }

    std::size_t messageSize = message->size();
    if (messageSize > SDL_MAX_UINT16) {
        DebugError("Tried to send a too-large message. Size: %u, max: %u", messageSize, SDL_MAX_UINT16);
    }

    int bytesSent = SDLNet_TCP_Send(socket, message->data(), messageSize);
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

ReceiveResult AM::Peer::receiveBytes(Uint16 numBytes, bool checkSockets)
{
    if (!bIsConnected) {
        return {NetworkResult::Disconnected, nullptr};
    }
    else if (checkSockets) {
        // Poll to see if there's data
        int numReady = SDLNet_CheckSockets(*set, 0);
        if (numReady == -1) {
            DebugInfo("Error while checking sockets: %s", SDLNet_GetError());
            // Most of the time this is a system error, where perror might help.
            perror("SDLNet_CheckSockets");
        }
    }

    if (!SDLNet_SocketReady(socket)) {
        return {NetworkResult::NoWaitingData, nullptr};
    }
    else {
        return receiveBytesWait(numBytes);
    }
}

ReceiveResult AM::Peer::receiveBytesWait(Uint16 numBytes)
{
    if (!bIsConnected) {
        return {NetworkResult::Disconnected, nullptr};
    }
    else if (numBytes > MAX_MESSAGE_SIZE) {
        DebugError(
            "Tried to receive too large of a message. messageSize: %u, MaxSize: %u",
            numBytes, MAX_MESSAGE_SIZE);
    }

    int result = SDLNet_TCP_Recv(socket, &messageBuffer, numBytes);
    if (result <= 0) {
        // Disconnected
        bIsConnected = false;
        return {NetworkResult::Disconnected, nullptr};
    }
    else if (result < numBytes) {
        DebugError("Didn't receive all the bytes in one chunk."
                   "Need to add logic for this scenario.");
    }

    // TODO: Can we receive directly into the BinaryBuffer instead of copying?
    return {NetworkResult::Success,
            std::make_unique<std::vector<Uint8>>(messageBuffer.begin()
                , (messageBuffer.begin() + numBytes))};
}

ReceiveResult AM::Peer::receiveMessage(bool checkSockets)
{
    if (!bIsConnected) {
        return {NetworkResult::Disconnected, nullptr};
    }
    else if (checkSockets) {
        // Poll to see if there's data
        int numReady = SDLNet_CheckSockets(*set, 0);
        if (numReady == -1) {
            DebugInfo("Error while checking sockets: %s", SDLNet_GetError());
            // Most of the time this is a system error, where perror might help.
            perror("SDLNet_CheckSockets");
        }
    }

    if (!SDLNet_SocketReady(socket)) {
        return {NetworkResult::NoWaitingData, nullptr};
    }
    else {
        return receiveMessageWait();
    }
}

ReceiveResult AM::Peer::receiveMessageWait()
{
    if (!bIsConnected) {
        return {NetworkResult::Disconnected, nullptr};
    }

    // Receive the size.
    Uint8 sizeBuf[sizeof(Uint16)];
    if (SDLNet_TCP_Recv(socket, sizeBuf, sizeof(Uint16)) <= 0) {
        // Disconnected
        bIsConnected = false;
        return {NetworkResult::Disconnected, nullptr};
    }

    // The number of bytes in the upcoming message.
    Uint16 messageSize = _SDLNet_Read16(sizeBuf);

    if (messageSize > MAX_MESSAGE_SIZE) {
        DebugError(
            "Tried to receive too large of a message. messageSize: %u, MaxSize: %u",
            messageSize, MAX_MESSAGE_SIZE);
    }

    int result = SDLNet_TCP_Recv(socket, &messageBuffer, messageSize);
    if (result <= 0) {
        // Disconnected
        bIsConnected = false;
        return {NetworkResult::Disconnected, nullptr};
    }
    else if (result < messageSize) {
        DebugError("Didn't receive all the bytes in one chunk."
                   "Need to add logic for this scenario.");
    }

    return {NetworkResult::Success,
            std::make_unique<std::vector<Uint8>>(messageBuffer.begin(),
                (messageBuffer.begin() + messageSize))};
}

