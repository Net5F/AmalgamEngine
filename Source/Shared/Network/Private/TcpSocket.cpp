#include "TcpSocket.h"
#include <SDL2/SDL_net.h>
#include "Debug.h"

namespace AM
{
TcpSocket::TcpSocket(Uint16 inPort)
: ip("")
, port(inPort)
{
    // We explicitly guard against this since we use port == 0 as a flag.
    if (port == 0) {
        DebugError("Tried to use port 0.");
    }

    IPaddress ip;
    if (SDLNet_ResolveHost(&ip, nullptr, port) == -1) {
        DebugError("Could not resolve host: %s", SDLNet_GetError());
    }

    socket = SDLNet_TCP_Open(&ip);
    if (socket == nullptr) {
        DebugError("Could not open TCP socket: %s", SDLNet_GetError());
    }
}

TcpSocket::TcpSocket(TCPsocket inSdlSocket)
: socket(inSdlSocket)
, ip("")
, port(0)
{
}

TcpSocket::TcpSocket(std::string inIp, Uint16 inPort)
: ip(inIp)
, port(inPort)
{
    // We explicitly guard against this since we use port == 0 as a flag.
    if (port == 0) {
        DebugError("Tried to use port 0.");
    }

    IPaddress ipObj;

    if (SDLNet_ResolveHost(&ipObj, ip.c_str(), port) == -1) {
        DebugError("Could not resolve host: %s", SDLNet_GetError());
    }

    socket = SDLNet_TCP_Open(&ipObj);
    if (socket == nullptr) {
        DebugError("Could not open TCP socket: %s", SDLNet_GetError());
    }
}

TcpSocket::~TcpSocket()
{
    SDLNet_TCP_Close(socket);
}

int TcpSocket::send(const void* dataBuffer, int len)
{
    return SDLNet_TCP_Send(socket, dataBuffer, len);
}

int TcpSocket::receive(void* dataBuffer, int maxLen)
{
    return SDLNet_TCP_Recv(socket, dataBuffer, maxLen);
}

bool TcpSocket::isReady()
{
    return SDLNet_SocketReady(socket);
}

std::unique_ptr<TcpSocket> TcpSocket::accept()
{
    TCPsocket newSocket = SDLNet_TCP_Accept(socket);
    if (newSocket == nullptr) {
        return nullptr;
    }
    else {
        return std::make_unique<TcpSocket>(newSocket);
    }
}

std::string TcpSocket::getAddress()
{
    if (ip.empty() && (port != 0)) {
        // Listener socket.
        DebugError("Tried to call getAddress on a listener socket.");
    }
    else if (port == 0) {
        // Socket was received through a listener and hasn't yet retrieved its
        // address.
        IPaddress* remoteIP = SDLNet_TCP_GetPeerAddress(socket);
        if (remoteIP == nullptr) {
            DebugError("Failed to get peer address: %s", SDLNet_GetError());
        }
        else {
            // Successfully got the address, save it in our members.
            ip = std::to_string(remoteIP->host);
            port = remoteIP->port;
        }
    }

    return ip + std::to_string(port);
}

TCPsocket TcpSocket::getUnderlyingSocket() const
{
    return socket;
}

} // End namespace AM
