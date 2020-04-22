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
    Acceptor(std::string inHostIP, unsigned int inPort);

    ~Acceptor();

    std::shared_ptr<Peer> accept();

private:
    std::string hostIP;
    Uint16 port;
    TCPsocket socket;
};


} /* End namespace AM */

#endif /* End ACCEPTOR_H_ */
