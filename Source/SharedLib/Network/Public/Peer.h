#pragma once

#include "NetworkDefs.h"
#include "SocketSet.h"
#include "TcpSocket.h"
#include <memory>
#include <array>
#include <atomic>

namespace AM
{
/**
 * Represents a network peer for communication.
 *
 * This class helps us interact with sockets in the ways that we usually like
 * to. If different behavior is needed, TcpSocket/SocketSet should be used
 * directly.
 */
class Peer
{
public:
    /**
     * Initiates a TCP connection that the other side can then accept.
     * (e.g. the client connecting to the server)
     */
    static std::unique_ptr<Peer> initiate(const std::string& serverIP,
                                          unsigned int serverPort);

    /**
     * Constructor for when you only need 1 peer (client connecting to server,
     * anyone connecting to chat server.)
     * Takes ownership of the given socket and adds it as the only member of a
     * socket set.
     */
    Peer(TcpSocket&& inSocket);

    /**
     * Constructor for when you need a set of peers (server connecting to
     * clients).
     * Takes ownership of the given socket and adds it to the given set.
     */
    Peer(TcpSocket&& inSocket, const std::shared_ptr<SocketSet>& inSet);

    /**
     * Removes the socket from the set.
     */
    ~Peer();

    /**
     * Returns false if the client was at some point found to be disconnected,
     * else true.
     */
    bool isConnected() const;

    /**
     * Sends the data in the given buffer to this Peer.
     *
     * Will error if the buffer size is larger than MAX_WIRE_SIZE.
     *
     * @return Disconnected if the peer was found to be disconnected, else
     *         Success.
     */
    NetworkResult send(const BinaryBufferSharedPtr& buffer);

    /**
     * Sends the data in the given buffer to this Peer.
     *
     * Will error if numBytes is larger than MAX_WIRE_SIZE.
     *
     * @return Disconnected if the peer was found to be disconnected, else
     *         Success.
     */
    NetworkResult send(const Uint8* buffer, std::size_t numBytesToSend);

    /**
     * Returns true if this socket has data waiting.
     *
     * @param checkSockets If true, will call checkSockets() before checking
     *                     socketReady(). Set this to false if you're going to
     *                     call checkSockets() yourself.
     */
    bool isReady(bool checkSockets);

    /**
     * Tries to receive bytes over the network.
     *
     * @param buffer The buffer to fill with data, if any was received.
     * @param numBytes The number of bytes to receive.
     * @return The number of received bytes, or -1 if this peer was disconnected.
     */
    int receiveBytes(Uint8* buffer, std::size_t numBytes);

    /**
     * Returns the requested number of bytes, waiting if they're not yet
     * available.
     *
     * @param buffer  The buffer to fill with data, if any was received.
     * @param numBytes  The number of bytes to receive.
     * @return The number of received bytes, or -1 if this peer was disconnected.
     */
    int receiveBytesWait(Uint8* buffer, std::size_t numBytes);

private:
    /** The socket for this peer. Must be a unique_ptr so we can move without
        copying. */
    TcpSocket socket;

    /** The set that this peer belongs to.
        Must be a shared_ptr since we may or may not allocate it ourselves
        depending on which constructor is called. */
    std::shared_ptr<SocketSet> set;

    /** Tracks whether or not this peer is connected. Is set to false if a
        disconnect was detected when trying to send or receive. */
    std::atomic<bool> bIsConnected;
};

} /* End namespace AM */
