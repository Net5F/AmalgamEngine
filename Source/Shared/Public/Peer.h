#ifndef PEER_H_
#define PEER_H_

#include "NetworkDefs.h"
#include "SocketSet.h"
#include "TcpSocket.h"
#include <memory>
#include <array>

namespace AM
{

/**
 * Represents a network peer for communication.
 * TODO: Peer/acceptor seem like a redundant layer and should probably be removed.
 *       A Client/Server class and the SocketSet/TcpSocket classes should be able to
 *       cleanly handle all the responsibilities.
 */
class Peer
{
public:
    /** Largest message we'll accept. Kept at 1450 for now to try to avoid IP fragmentation.
     *  Can rethink if we need larger. */
    static constexpr unsigned int MAX_MESSAGE_SIZE = 1450;

    /**
     * Initiates a TCP connection that the other side can then accept.
     * (e.g. the client connecting to the server)
     */
    static std::unique_ptr<Peer> initiate(std::string serverIP, unsigned int serverPort);

    /**
     * Constructor for when you only need 1 peer (client connecting to server, anyone
     * connecting to chat server.)
     * Constructs a socket set for this peer to use and adds the socket to it.
     */
    Peer(std::unique_ptr<TcpSocket> inSocket);

    /**
     * Constructor for when you need a set of peers (server connecting to clients).
     * Adds the socket to the given set.
     */
    Peer(std::unique_ptr<TcpSocket> inSocket, const std::shared_ptr<SocketSet>& inSet);

    /**
     * Removes the socket from the set.
     */
    ~Peer();

    /**
     * Returns false if the client was at some point found to be disconnected, else true.
     */
    bool isConnected() const;

    /**
     * Sends a message to this Peer.
     * Will error if the message size is larger than a Uint16 can hold.
     * @return Disconnected if the peer was found to be disconnected, else Success.
     */
    NetworkResult send(const BinaryBufferSharedPtr& message);

    /**
     * Tries to receive bytes over the network.
     *
     * @param checkSockets  If true, will call CheckSockets() before checking
     *                      SocketReady(). Set this to false if you're going to call
     *                      CheckSockets() yourself.
     * @param numBytes  The number of bytes to receive.
     * @return An appropriate ReceiveResult if the receive failed, else a ReceiveResult with
     *         result == Success and data in the message field.
     */
    ReceiveResult receiveBytes(Uint16 numBytes, bool checkSockets);

    /**
     * Returns the requested number of bytes, waiting if they're not yet available.
     * @param numBytes  The number of bytes to receive.
     * @return A message.
     */
    ReceiveResult receiveBytesWait(Uint16 numBytes);

    /**
     * Tries to receive a {size, message} pair over the network.
     *
     * @param checkSockets  If true, will call CheckSockets() before checking
     *                      SocketReady(). Set this to false if you're going to call
     *                      CheckSockets() yourself.
     * @return An appropriate ReceiveResult if the receive failed, else a ReceiveResult with
     *         result == Success and data in the message field.
     */
    ReceiveResult receiveMessage(bool checkSockets);

    /**
     * Receives a {size, message} pair and returns a message, waiting if the data is
     * not yet available.
     * @return A message.
     */
    ReceiveResult receiveMessageWait();

private:
    /** The socket for this peer. Must be a unique_ptr so we can move without copying. */
    std::unique_ptr<TcpSocket> socket;
    /** The set that this peer belongs to. Must be a shared_ptr since we may or may not
        allocate it ourselves depending on which constructor is called. */
    std::shared_ptr<SocketSet> set;

    /**
     * Tracks whether or not this peer is connected. Is set to false if a disconnect
     * was detected when trying to send or receive.
     */
    bool bIsConnected;
};

} /* End namespace AM */

#endif /* End PEER_H_ */
