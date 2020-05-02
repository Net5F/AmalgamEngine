#include "Acceptor.h"
#include <iostream>

namespace AM
{

Acceptor::Acceptor(unsigned int inPort, std::shared_ptr<SDLNet_SocketSet> inClientSet)
: port(inPort)
, clientSet(inClientSet)
{
    IPaddress ip;
    if (SDLNet_ResolveHost(&ip, NULL, port)) {
        std::cerr << SDLNet_GetError() << std::endl;
    }

    socket = SDLNet_TCP_Open(&ip);
    if (!socket) {
        std::cerr << SDLNet_GetError() << std::endl;
    }
}

AM::Acceptor::~Acceptor()
{
    SDLNet_TCP_Close(socket);
}

std::shared_ptr<Peer> AM::Acceptor::accept()
{
    TCPsocket client = SDLNet_TCP_Accept(socket);
    if (client) {
        return std::make_shared<Peer>(client, clientSet);
    }
    else {
        return nullptr;
    }
}

} // namespace AM
