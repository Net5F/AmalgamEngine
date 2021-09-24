#pragma once

#include "NetworkDefs.h"
#include "readerwriterqueue.h"
#include <memory>

namespace AM
{
class ConnectionResponse;
class EntityUpdate;

namespace Client
{
class Network;
class NpcUpdateMessage;

/**
 * Handles received messages, queueing them for the relevant systems and
 * updating Network data as necessary.
 */
class MessageHandler
{
public:
    MessageHandler(Network& inNetwork);

    template <typename T>
    static moodycamel::BlockingReaderWriterQueue<std::shared_ptr<T>>& getQueue()
    {
        static moodycamel::BlockingReaderWriterQueue<std::shared_ptr<T>> queue;
        return queue;
    }

    template <typename T>
    void handle(const std::shared_ptr<T>& message)
    {
        // Push the message into its queue.
        getQueue<T>().enqueue(message);
    }

    void handleConnectionResponse(BinaryBuffer& messageRecBuffer,
                                  Uint16 messageSize);

    void handleEntityUpdate(BinaryBuffer& messageRecBuffer, Uint16 messageSize);

    void handleExplicitConfirmation(BinaryBuffer& messageRecBuffer, Uint16 messageSize);

    /**
     * These queues store received messages that are waiting to be consumed.
     * TODO: These should be replaced with callbacks. Each system that cares
     *       about a message could register for it through a Network function.
     *       The handler function would pass it through the callback, to
     *       then be queued in a threadsafe queue in the system.
     */
    using ConnectionResponseQueue = moodycamel::BlockingReaderWriterQueue<
        std::unique_ptr<ConnectionResponse>>;
    ConnectionResponseQueue connectionResponseQueue;

    using PlayerUpdateQueue = moodycamel::BlockingReaderWriterQueue<
        std::shared_ptr<const EntityUpdate>>;
    PlayerUpdateQueue playerUpdateQueue;

    using NpcUpdateQueue
        = moodycamel::BlockingReaderWriterQueue<NpcUpdateMessage>;
    NpcUpdateQueue npcUpdateQueue;

private:
    Network& network;
};

} // End namespace Client
} // End namespace AM
