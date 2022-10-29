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
 * Owns a listener socket and provides an interface for accepting new Peers.
 */
class Acceptor
{
public:
    Acceptor(Uint16 port, const std::shared_ptr<SocketSet>& inClientSet);

    ~Acceptor();

    /**
     * If a peer is waiting to connect, opens a connection to the peer and
     * adds it to the clientSet.
     *
     * @return A pointer to a new peer, if one was waiting. Else, nullptr.
     */
    std::unique_ptr<Peer> accept();

    /**
     * If a peer is waiting to connect, opens a connection to the peer and
     * immediately closes it.
     *
     * Use this to close connections when you're at maximum capacity.
     *
     * @return true if a peer was waiting, else false.
     */
    bool reject();

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
