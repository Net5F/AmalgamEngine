#ifndef NETWORK_H
#define NETWORK_H

#include "SharedDefs.h"
#include "SDL_thread.h"
#include <string>
#include <memory>
#include <cstddef>
#include <unordered_map>
#include <atomic>
#include "readerwriterqueue.h"

struct _SDLNet_SocketSet;
typedef struct _SDLNet_SocketSet* SDLNet_SocketSet;

namespace AM
{

class Acceptor;
class Peer;

namespace Server
{

class Network
{
public:
    static constexpr unsigned int MAX_CLIENTS = 100;
    static constexpr unsigned int MAX_QUEUED_INPUT_MESSAGES = 1000;
    static constexpr unsigned int MAX_QUEUED_NEW_CLIENTS = 100;

    Network();

    virtual ~Network();

    /**
     * Sends message over the network.
     * @return false if the send failed, else true.
     */
    bool send(std::shared_ptr<Peer> client, BinaryBufferSharedPtr message);

    /**
     * Sends message to all connected clients.
     * @return false if any send failed, else true.
     */
    bool sendToAll(BinaryBufferSharedPtr message);

    /**
     * Returns the number of waiting input messages.
     * Allows you to take a snapshot so you aren't looping on an ever-growing queue.
     */
    unsigned int getNumInputMessagesWaiting();

    /**
     * Returns a message if there are any in inputQueue.
     * @return A waiting message, else nullptr.
     */
    BinaryBufferPtr receiveInputMessage();

    /**
     * Accepts any new clients, pushing them into the newClientQueue.
     * Note: DOES NOT add new clients to the client map. You must tie them to an
     *       entityID through addClient().
     */
    void acceptNewClients();

    /**
     * Adds the given client to the client map at key entityID.
     */
    void addClient(EntityID entityID, std::shared_ptr<Peer> client);

    /**
     * @return A pointer to a new client if one is waiting, else nullptr.
     */
    std::shared_ptr<Peer> getNewClient();

    const std::shared_ptr<SDLNet_SocketSet> getClientSet();

    const std::unordered_map<EntityID, std::shared_ptr<Peer>>& getClients();

    std::atomic<bool> const* getExitRequestedPtr();

private:
    /**
     * Iterates through the clients and erases any that are disconnected.
     */
    void eraseDisconnectedClients();

    /**
     * Tries to retrieve a message from the server.
     * If successful, passes it to queueMessage().
     */
    static int pollForMessages(void* inNetwork);

    /**
     * Used by pollForMessages, checks for new messages and pushes them into their queues.
     */
    static void receiveClientMessages(
    Network* network, const std::shared_ptr<SDLNet_SocketSet> clientSet,
    const std::unordered_map<EntityID, std::shared_ptr<Peer>>& clients);

    /**
     * Pushes a message into the inputQueue.
     */
    void queueInputMessage(BinaryBufferPtr message);


    static const std::string SERVER_IP;
    static constexpr int SERVER_PORT = 41499;

    std::unique_ptr<Acceptor> acceptor;
    /** Stores new connected clients until the game ties them to an ID and
        adds them to the clients map. */
    moodycamel::ReaderWriterQueue<std::shared_ptr<Peer>> newClientQueue;

    /** The socket set used for all clients. Lets us do select()-like behavior,
        allowing our receive thread to not be constantly spinning. */
    std::shared_ptr<SDLNet_SocketSet> clientSet;
    /** Maps IDs to their connections. Allows the game to say "send this message
        to this entity" instead of needing to track the connection objects. */
    std::unordered_map<EntityID, std::shared_ptr<Peer>> clients;

    /** Stores input messages from clients. */
    moodycamel::ReaderWriterQueue<BinaryBufferPtr> inputQueue;

    /** Calls pollForMessages(). */
    SDL_Thread* receiveThreadPtr;
    /** Turn false to signal that the receive thread should end. */
    std::atomic<bool> exitRequested;
};

} // namespace Server
} // namespace AM

#endif /* NETWORK_H */
