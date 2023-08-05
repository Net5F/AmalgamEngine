#pragma once

#include "NetworkDefs.h"
#include "entt/fwd.hpp"
#include <memory>

namespace AM
{
class EventDispatcher;

namespace Server
{
class IMessageProcessorExtension;

/**
 * Processes received messages.
 *
 * If the message is relevant to the network layer, it's passed to a matching
 * function that contains all of its handling logic.
 *
 * If the message isn't relevant to the network layer, it's passed to a generic
 * function that pushes it straight down to the simulation layer.
 *
 * If the message isn't relevant to the engine at all, it's passed to the 
 * project.
 */
class MessageProcessor
{
public:
    MessageProcessor(EventDispatcher& inNetworkEventDispatcher);

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
    Sint64 processReceivedMessage(NetworkID netID, Uint8 messageType,
                                  Uint8* messageBuffer,
                                  std::size_t messageSize);

    /**
     * See extension member comment.
     */
    void setExtension(std::unique_ptr<IMessageProcessorExtension> inExtension);

private:
    //-------------------------------------------------------------------------
    // Handlers for messages relevant to the network layer.
    //-------------------------------------------------------------------------
    /**
     * Pushes nothing - Handled in network layer.
     * @return The tick number that the message contained.
     */
    Uint32 handleHeartbeat(Uint8* messageBuffer, std::size_t messageSize);

    /**
     * Pushes InputChangeRequest event.
     * @return The tick number that the message contained.
     */
    Uint32 handleInputChangeRequest(NetworkID netID, Uint8* messageBuffer,
                                    std::size_t messageSize);

    /** Pushes ChunkUpdateRequest event. */
    void handleChunkUpdateRequest(NetworkID netID, Uint8* messageBuffer,
                                  std::size_t messageSize);
    //-------------------------------------------------------------------------

    /** The network's event dispatcher. Used to send events to the subscribed
        queues. */
    EventDispatcher& networkEventDispatcher;

    /** If non-nullptr, contains the project's message processing extension
        functions.
        Allows the project to provide message processing code and have it be
        called at the appropriate time. */
    std::unique_ptr<IMessageProcessorExtension> extension;
};

} // End namespace Server
} // End namespace AM
