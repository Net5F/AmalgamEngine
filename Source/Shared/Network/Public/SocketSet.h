#ifndef SOCKETSET_H_
#define SOCKETSET_H_

#include <SDL2/SDL_net.h>
#include <memory>

namespace AM
{
class TcpSocket;

/**
 * Represents a set of sockets.
 * Wraps SDLNet's SocketSet in an RAII object interface.
 */
class SocketSet
{
public:
    /**
     * Allocates the socket set.
     *
     * @param maxSockets  The max sockets this socket set can hold.
     */
    SocketSet(int maxSockets);

    /**
     * Deallocates the socket set.
     */
    ~SocketSet();

    // Not copyable.
    SocketSet(const SocketSet& otherSet) = delete;
    SocketSet& operator=(const SocketSet& otherSet) = delete;

    /**
     * Adds the given socket to this set.
     */
    void addSocket(const TcpSocket& socket);

    /**
     * Removes the given socket from this set.
     */
    void remSocket(const TcpSocket& socket);

    /**
     * Checks all sockets in the set for activity.
     * If a non-zero timeout is given, will wait up to that long for activity.
     *
     * @param timeoutMs  The time in milliseconds to wait for activity.
     * @return The number of sockets with activity.
     */
    int checkSockets(unsigned int timeoutMs);

private:
    SDLNet_SocketSet set;

    /** The number of sockets currently in the set. */
    int numSockets;
};

} // End namespace AM

#endif /* End SOCKETSET_H_ */
