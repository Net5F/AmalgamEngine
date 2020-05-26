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
 * This class represents a single client and facilitates the organization
 * of our communication with them.
 */
class Client
{
public:
    Client(std::shared_ptr<Peer> inPeer);

    /**
     * Returns whether or not this client is connected.
     * Disconnects are detected while attempting to send or receive.
     */
    bool isConnected() const;

    /**
     * Attempts to send the given message over the network.
     */
    bool send(BinaryBufferSharedPtr message);

    /**
     * Attempts to send all queued messages over the network.
     */
    bool sendWaitingMessages();

    /**
     * Tries to receive a message from the Peer.
     * Note: It's expected that you called SDLNet_CheckSockets() on the outside-managed
     *       socket set before calling this.
     */
    BinaryBufferSharedPtr receiveMessage();

    /**
     * Returns the number of messages waiting in sendQueue.
     * The return type is Uint8 because it needs to fit in 1 byte of a message.
     */
    Uint8 getWaitingMessageCount() const;

private:
    std::shared_ptr<Peer> peer;
    std::deque<BinaryBufferSharedPtr> sendQueue;
};

} // End namespace Server
} // End namespace AM

#endif /* End CLIENT_H_ */
