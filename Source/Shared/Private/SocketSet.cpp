#include "SocketSet.h"
#include "TcpSocket.h"
#include "Debug.h"

namespace AM
{

SocketSet::SocketSet(int maxSockets)
: numSockets(0)
{
    set = SDLNet_AllocSocketSet(maxSockets);
    if (set == nullptr) {
        DebugError("Error allocating socket set: %s", SDLNet_GetError());
    }
}

SocketSet::~SocketSet()
{
    SDLNet_FreeSocketSet(set);
    set = nullptr;
}

void SocketSet::addSocket(const TcpSocket& socket)
{
    int numAdded = SDLNet_TCP_AddSocket(set, socket.getUnderlyingSocket());
    if (numAdded < 1) {
        DebugError("Error while adding socket: %s", SDLNet_GetError());
    }
    else if (numAdded != 1) {
        DebugError("Unexpected value returned from AddSocket. numAdded: %d", numAdded);
    }
    else {
        numSockets += numAdded;
    }
}

void SocketSet::remSocket(const TcpSocket& socket)
{
    SDLNet_TCP_DelSocket(set, socket.getUnderlyingSocket());
}

int SocketSet::checkSockets(unsigned int timeoutMs)
{
    int numReady = SDLNet_CheckSockets(set, timeoutMs);
    if (numReady == -1) {
        DebugInfo("Error while checking sockets: %s", SDLNet_GetError());
        // Most of the time this is a system error, where perror might help.
        perror("SDLNet_CheckSockets");
    }

    return numReady;
}

} // End namespace AM
