#include "TcpSocket.h"
#include <SDL_net.h>
#include "Log.h"

namespace AM
{
TcpSocket::TcpSocket()
: socket{nullptr}
, ip{""}
, port{0}
{
}

TcpSocket::TcpSocket(TCPsocket inSdlSocket)
: socket(inSdlSocket)
, ip{""}
, port{0}
{
}

TcpSocket::~TcpSocket()
{
    close();
}

TcpSocket::TcpSocket(TcpSocket&& otherSocket) noexcept
: socket{otherSocket.socket}
, ip{otherSocket.ip}
, port{otherSocket.port}
{
    otherSocket.socket = nullptr;
    otherSocket.ip = "";
    otherSocket.port = 0;
}

bool TcpSocket::openAsListener(Uint16 portToListenOn)
{
    // We explicitly guard against this since we use port == 0 as a flag.
    if (portToListenOn == 0) {
        LOG_FATAL("Tried to listen on port 0.");
    }

    IPaddress ipObj;
    if (SDLNet_ResolveHost(&ipObj, nullptr, portToListenOn) == -1) {
        LOG_INFO("Could not resolve host: %s", SDLNet_GetError());
        return false;
    }

    socket = SDLNet_TCP_Open(&ipObj);
    if (socket == nullptr) {
        LOG_INFO("Could not open TCP socket: %s", SDLNet_GetError());
        return false;
    }

    return true;
}

bool TcpSocket::openConnectionTo(std::string ip, Uint16 port)
{
    // We explicitly guard against this since we use port == 0 as a flag.
    if (port == 0) {
        LOG_FATAL("Tried to use port 0.");
    }

    IPaddress ipObj;
    if (SDLNet_ResolveHost(&ipObj, ip.c_str(), port) == -1) {
        LOG_INFO("Could not resolve host: %s", SDLNet_GetError());
        return false;
    }

    socket = SDLNet_TCP_Open(&ipObj);
    if (socket == nullptr) {
        LOG_INFO("Could not open TCP socket: %s", SDLNet_GetError());
        return false;
    }

    return true;
}

void TcpSocket::close()
{
    if (socket != nullptr) {
        SDLNet_TCP_Close(socket);
        socket = nullptr;
    }
}

bool TcpSocket::isOpen()
{
    return (socket != nullptr);
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

TcpSocket TcpSocket::accept()
{
    TCPsocket newSocket{SDLNet_TCP_Accept(socket)};
    if (newSocket != nullptr) {
        return TcpSocket{newSocket};
    }
    else {
        return TcpSocket{};
    }
}

std::string TcpSocket::getAddress()
{
    if (ip.empty() && (port != 0)) {
        // Listener socket.
        LOG_FATAL("Tried to call getAddress on a listener socket.");
    }
    else if (port == 0) {
        // Socket was received through a listener and hasn't yet retrieved its
        // address.
        IPaddress* remoteIP{SDLNet_TCP_GetPeerAddress(socket)};
        if (remoteIP == nullptr) {
            LOG_FATAL("Failed to get peer address: %s", SDLNet_GetError());
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
