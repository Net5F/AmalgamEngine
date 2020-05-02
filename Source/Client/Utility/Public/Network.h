#ifndef NETWORK_H
#define NETWORK_H

#include "SharedDefs.h"
#include <string>
#include <memory>
#include <atomic>
#include <thread>
#include "readerwriterqueue.h"

namespace AM
{

class Peer;

namespace Client
{

class Network
{
public:
    Network();

    virtual ~Network();

    bool connect();

    /**
     * Sends bytes over the network.
     */
    bool send(BinaryBufferSharedPtr message);

    /**
     * Returns a message if there are any in the queue.
     * @return A waiting message, else nullptr.
     */
    BinaryBufferPtr receive();

    /**
     * Thread function, started from connect().
     * Tries to retrieve a message from the server.
     * If successful, passes it to queueMessage().
     */
    static int pollForMessages(void* inNetwork);

    /**
     * Pushes a message into the receiveQueue.
     */
    void queueMessage(BinaryBufferPtr message);

    std::shared_ptr<Peer> getServer();

    std::atomic<bool> const* getExitRequestedPtr();

private:
    static const std::string SERVER_IP;
    static constexpr int SERVER_PORT = 41499;

    std::shared_ptr<Peer> server;

    /** Calls pollForMessages(). */
    std::thread receiveThreadObj;
    /** Turn false to signal that the receive thread should end. */
    std::atomic<bool> exitRequested;

    /** Stores messages after they're asynchronously received. */
    moodycamel::ReaderWriterQueue<BinaryBufferPtr> receiveQueue;
};

} // namespace Client
} // namespace AM

#endif /* NETWORK_H */
