#include "Peer.h"
#include <SDL_stdinc.h>
#include "Debug.h"

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
: peerIsConnected(false)
, set(nullptr)
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

    peerIsConnected = true;
}

AM::Peer::Peer(TCPsocket inSocket, std::shared_ptr<SDLNet_SocketSet> inSet)
: peerIsConnected(false)
, set(inSet)
{
    if (!(*set)) {
        DebugInfo("Tried to init Peer with bad set.");
    }

    int numAdded = SDLNet_TCP_AddSocket(*set, inSocket);
    if (numAdded < 1) {
        DebugInfo("Error while adding socket: %s", SDLNet_GetError());
    }

    socket = inSocket;

    peerIsConnected = true;
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

bool AM::Peer::isConnected()
{
    return peerIsConnected;
}

bool AM::Peer::sendMessage(BinaryBufferSharedPtr message)
{
    Uint8 size_buf[sizeof(Uint32)];
    std::size_t size = message->size();
    _SDLNet_Write32(size, size_buf);

    unsigned int bytesSent = SDLNet_TCP_Send(socket, size_buf, sizeof(Uint32));
    if (bytesSent < sizeof(Uint32)) {
        return false;
    }

    unsigned int messageSize = message->size();
    bytesSent = SDLNet_TCP_Send(socket, message->data(), messageSize);
    if (bytesSent < messageSize) {
        return false;
    }
    else {
        return true;
    }
}

BinaryBufferPtr AM::Peer::receiveMessage(bool checkSockets)
{
    if (checkSockets) {
        // Poll to see if there's data
        int numReady = SDLNet_CheckSockets(*set, 0);
        if (numReady == -1) {
            DebugInfo("Error while checking sockets: %s", SDLNet_GetError());
            // Most of the time this is a system error, where perror might help.
            perror("SDLNet_CheckSockets");
        }
    }

    if (!SDLNet_SocketReady(socket)) {
        return nullptr;
    }
    else {
        return receiveMessageWait();
    }
}

BinaryBufferPtr AM::Peer::receiveMessageWait()
{
    // Receive the header (single byte, says the size of the upcoming message.)
    Uint8 sizeBuf[sizeof(Uint32)];
    if (SDLNet_TCP_Recv(socket, sizeBuf, sizeof(Uint32)) <= 0) {
        // Disconnected
        peerIsConnected = false;
        return nullptr;
    }

    // The number of bytes in the upcoming message.
    Uint32 messageSize = _SDLNet_Read32(sizeBuf);

    if (messageSize > MAX_MESSAGE_SIZE) {
        DebugInfo("Tried to receive too large of a message. messageSize: %u, MaxSize: %u",
            messageSize, MAX_MESSAGE_SIZE);
        return nullptr;
    }

    if (SDLNet_TCP_Recv(socket, &messageBuffer, messageSize) <= 0) {
        // Disconnected
        peerIsConnected = false;
        return nullptr;
    }

    return std::make_unique<std::vector<Uint8>>(messageBuffer.begin()
    , (messageBuffer.begin() + messageSize));
}

