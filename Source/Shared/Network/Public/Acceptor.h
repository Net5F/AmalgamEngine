#ifndef ACCEPTOR_H_
#define ACCEPTOR_H_

#include "Peer.h"
#include "SocketSet.h"
#include "TcpSocket.h"
#include <SDL_net.h>
#include <memory>
#include <string>

namespace AM
{
/**
 * This class owns a listener socket and can accept new Peers.
 * TODO: Peer/acceptor seem like a redundant layer and should probably be
 * removed. A Client/Server class and the SocketSet/TcpSocket classes should be
 * able to cleanly handle all the responsibilities.
 */
class Acceptor
{
public:
    Acceptor(Uint16 port, const std::shared_ptr<SocketSet>& inClientSet);

    ~Acceptor();

    std::unique_ptr<Peer> accept();

private:
    /** Our listener socket. */
    TcpSocket socket;

    /** The set that we use to check if our socket has activity. */
    SocketSet listenerSet;

    /** A pointer to the set to add accepted clients to. */
    std::shared_ptr<SocketSet> clientSet;
};

} /* End namespace AM */

#endif /* End ACCEPTOR_H_ */
