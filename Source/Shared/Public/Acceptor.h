#ifndef ACCEPTOR_H_
#define ACCEPTOR_H_

#include "Peer.h"
#include <SDL2/SDL_net.h>
#include <memory>
#include <string>

namespace AM
{

/**
 *
 */
class Acceptor
{
public:
    Acceptor(unsigned int inPort, std::shared_ptr<SDLNet_SocketSet> inClientSet);

    ~Acceptor();

    std::unique_ptr<Peer> accept();

private:
    std::string hostIP;
    Uint16 port;
    TCPsocket socket;

    /** A pointer to the set to add accepted clients to. Typically owned by the Network. */
    std::shared_ptr<SDLNet_SocketSet> clientSet;
};


} /* End namespace AM */

#endif /* End ACCEPTOR_H_ */
