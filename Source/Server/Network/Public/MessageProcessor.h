#pragma once

#include "NetworkDefs.h"
#include "entt/fwd.hpp"
#include <memory>

namespace AM
{
class EventDispatcher;

namespace Server
{
class Network;
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
     * Deserializes and handles received messages.
     *
     * See class comment.
     *
     * @param netID  The network ID of the client that the message came from.
     * @param messageType  The type of the received message.
     * @param messageBuffer A buffer containing a serialized message, starting
     *                      at index 0.
     * @param messageSize  The length in bytes of the message in messageBuffer.
     *
     * @return If the message corresponds to a particular simulation tick,
     *         returns that tick number. If not, returns -1.
     */
    Sint64 processReceivedMessage(NetworkID netID, MessageType messageType,
                                BinaryBuffer& messageBuffer,
                                unsigned int messageSize);

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
    template<typename T>
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
    template<typename T>
    void pushEventSharedPtr(BinaryBuffer& messageBuffer,
                            unsigned int messageSize);

    //-------------------------------------------------------------------------
    // Handlers for messages relevant to the network layer.
    //-------------------------------------------------------------------------
    /**
     * Pushes nothing - Handled in network layer.
     * @return The tick number that the message contained.
     */
    Uint32 handleHeartbeat(BinaryBuffer& messageBuffer, unsigned int messageSize);

    /**
     * @return The tick number that the message contained.
     */
    Uint32 handleClientInput(NetworkID netID, BinaryBuffer& messageBuffer, unsigned int messageSize);
    //-------------------------------------------------------------------------

    /** The network's event dispatcher. Used to send events to the subscribed
        queues. */
    EventDispatcher& dispatcher;
};

} // End namespace Server
} // End namespace AM
