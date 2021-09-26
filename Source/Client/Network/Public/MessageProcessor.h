#pragma once

#include "NetworkDefs.h"
#include "entt/fwd.hpp"
#include <memory>

namespace AM
{
class EventDispatcher;

namespace Client
{

/**
 * Processes received messages.
 *
 * If the message is relevant to the network layer, it's passed to a matching
 * function that contains all of its handling logic.
 *
 * If the message isn't relevant to the network layer, it's passed to a generic
 * function that pushes it straight down to the simulation layer.
 */
class MessageProcessor
{
public:
    MessageProcessor(EventDispatcher& inDispatcher);

    /**
     * Pushes a message into the appropriate queue, based on its contents.
     *
     * @param messageType  The type of the received message to process.
     * @param messageBuffer A buffer containing a serialized message, starting
     *                      at index 0.
     * @param messageSize  The length in bytes of the message in messageBuffer.
     */
    void processReceivedMessage(MessageType messageType
                                , BinaryBuffer& messageBuffer, unsigned int messageSize);

private:
    /**
     * Pushes an event straight out to the simulation layer. Used when the
     * message type doesn't require any network-level processing.
     *
     * Deserializes the message to type T and pushes it into the Network's
     * dispatcher.
     *
     * The event can be received in a system using EventQueue<T>.
     */
    template <typename T>
    void pushEvent(BinaryBuffer& messageBuffer, unsigned int messageSize);

    /**
     * Similar to pushEvent(), but allocates the event to the heap through a
     * std::shared_ptr.
     *
     * Used for large events or events with internal allocations, where the
     * cost of allocating once is lower than copying.
     *
     * The event can be received in a system using
     * EventQueue<std::shared_ptr<const T>>.
     */
    template <typename T>
    void pushEventSharedPtr(BinaryBuffer& messageBuffer, unsigned int messageSize);

    //-------------------------------------------------------------------------
    // Handlers for messages relevant to the network layer.
    //-------------------------------------------------------------------------
    /** Pushes ExplicitConfirmation event. */
    void handleExplicitConfirmation(BinaryBuffer& messageBuffer, Uint16 messageSize);

    /** Pushes ConnectionResponse event. */
    void handleConnectionResponse(BinaryBuffer& messageBuffer,
                                  Uint16 messageSize);

    /** Pushes std::shared_ptr<const EntityUpdate> event. **/
    void handleEntityUpdate(BinaryBuffer& messageBuffer, Uint16 messageSize);
    //-------------------------------------------------------------------------

    /** The network's event dispatcher. Used to send events to the subscribed
        queues. */
    EventDispatcher& dispatcher;

    /** Local copy of the playerEntity so we can tell if we got a player
        message. */
    entt::entity playerEntity;
};

} // End namespace Client
} // End namespace AM
