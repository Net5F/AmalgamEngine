#ifndef CLIENT_H_
#define CLIENT_H_

#include "GameDefs.h"
#include "NetworkDefs.h"
#include <memory>
#include <queue>

namespace AM
{

class Peer;

namespace Server
{

/**
 * This class represents a single client and facilitates the organization of our
 * communication with them. It's effectively an adapter for Peer with an added outgoing queue.
 */
class Client
{
public:
    Client(const std::shared_ptr<Peer>& inPeer);

    /**
     * Immediately sends the given header to this Peer.
     *
     * Could technically send any message, but our use case is only for headers. All other
     * messages should be queued through queueMessage.
     *
     * Will error if the message size is larger than a Uint16 can hold.
     * @return Disconnected if the peer was found to be disconnected, else Success.
     */
    NetworkResult sendHeader(const BinaryBufferSharedPtr& header);

    /**
     * Queues a message to be sent the next time sendWaitingMessages is called.
     */
    void queueMessage(const BinaryBufferSharedPtr& message);

    /**
     * Attempts to send all queued messages over the network.
     * @return false if the client was found to be disconnected, else true.
     */
    NetworkResult sendWaitingMessages();

    /**
     * Tries to receive a message from the Peer.
     * Note: It's expected that you called SDLNet_CheckSockets() on the outside-managed
     *       socket set before calling this.
     *
     * @return An appropriate ReceiveResult if the receive failed, else a ReceiveResult with
     *         result == Success and data in the message field.
     */
    ReceiveResult receiveMessage();

    /**
     * Returns the number of messages waiting in sendQueue.
     * The return type is Uint8 because it needs to fit in 1 byte of a message.
     */
    Uint8 getWaitingMessageCount() const;

    bool isConnected();

private:
    std::shared_ptr<Peer> peer;
    std::deque<BinaryBufferSharedPtr> sendQueue;
};

} // End namespace Server
} // End namespace AM

#endif /* End CLIENT_H_ */
