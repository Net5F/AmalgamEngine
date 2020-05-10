#ifndef NETWORK_H
#define NETWORK_H

#include "SharedDefs.h"
#include "Message_generated.h"
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

/**
 * The various types of flatbuffer message that we receive.
 * Enum class because it doesn't read well from other namespaces.
 */
enum class MessageType {
    ConnectionResponse,
    PlayerUpdate,
    NpcUpdate
};

class Network
{
public:
    Network();

    virtual ~Network();

    bool connect();

    /**
     * Registers an entity as being the player. Various systems will only apply to this entity.
     */
    void registerPlayerID(EntityID inPlayerID);

    /**
     * Sends bytes over the network.
     */
    bool send(BinaryBufferSharedPtr message);

    /**
     * Returns a message if there are any in the requested queue.
     * @return A waiting message, else nullptr.
     */
    BinaryBufferSharedPtr receive(MessageType type);

    /**
     * Thread function, started from connect().
     * Tries to retrieve a message from the server.
     * If successful, passes it to queueMessage().
     */
    static int pollForMessages(void* inNetwork);

    /**
     * Pushes a message into the appropriate queue, based on its contents.
     */
    void queueMessage(BinaryBufferSharedPtr messageBuffer);

    std::shared_ptr<Peer> getServer();

    std::atomic<bool> const* getExitRequestedPtr();

private:
    static const std::string SERVER_IP;
    static constexpr int SERVER_PORT = 41499;

    std::shared_ptr<Peer> server;

    /** Local copy of the playerID so we can tell if we got a player message. */
    EntityID playerID;

    /** Calls pollForMessages(). */
    std::thread receiveThreadObj;
    /** Turn false to signal that the receive thread should end. */
    std::atomic<bool> exitRequested;

    /** These queues store received messages that are waiting to be consumed. */
    typedef moodycamel::ReaderWriterQueue<BinaryBufferSharedPtr> MessageQueue;
    MessageQueue connectionResponseQueue;
    MessageQueue playerUpdateQueue;
    MessageQueue npcUpdateQueue;
};

} // namespace Client
} // namespace AM

#endif /* NETWORK_H */
