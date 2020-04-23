#include "Peer.h"
#include <SDL_stdinc.h>
#include <iostream>

using namespace AM;

std::unique_ptr<Peer> AM::Peer::initiate(std::string serverIP,
                                         unsigned int serverPort)
{
    IPaddress ip;

    if (SDLNet_ResolveHost(&ip, serverIP.c_str(), serverPort)) {
        std::cerr << "Could not resolve host" << std::endl;
    }

    TCPsocket socket = SDLNet_TCP_Open(&ip);
    if (!socket) {
        std::cerr << "Could not open socket" << std::endl;
    }

    return std::make_unique<Peer>(socket);
}

AM::Peer::Peer(TCPsocket inSocket)
: peerIsConnected(false)
{
    set = SDLNet_AllocSocketSet(1);
    if (!set) {
        std::cerr << "Error allocating socket set" << SDLNet_GetError() << std::endl;
    }

    int numAdded = SDLNet_TCP_AddSocket(set, inSocket);
    if (numAdded < 1) {
        std::cerr << "Error while adding socket." << SDLNet_GetError() << std::endl;
    }

    socket = inSocket;

    peerIsConnected = true;
}

AM::Peer::~Peer()
{
    SDLNet_FreeSocketSet(set);
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
    if (bytesSent < MESSAGE_HEADER_SIZE) {
        return false;
    }

    bytesSent = SDLNet_TCP_Send(socket, message->data(), message->size());
    if (bytesSent < MESSAGE_HEADER_SIZE) {
        return false;
    }
    else {
        return true;
    }
}

BinaryBufferPtr AM::Peer::receiveMessage()
{
    // Poll to see if there's data
    int numReady = SDLNet_CheckSockets(set, 0);
    if (numReady == -1) {
        std::cerr << "Error while checking sockets: " << SDLNet_GetError() << std::endl;
        // Most of the time this is a system error, where perror might help.
        perror("SDLNet_CheckSockets");
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
        std::cerr << "Tried to receive too large of a message." << std::endl;
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

