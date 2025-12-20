#pragma once

#include "IMessageProcessorExtension.h"
#include "entt/fwd.hpp"
#include <SDL_stdinc.h>
#include <memory>
#include <vector>
#include <span>
#include <atomic>

namespace AM
{
class EventDispatcher;

namespace Client
{
struct MessageProcessorContext;

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
    MessageProcessor(const MessageProcessorContext& inMessageProcessorContext);

    /**
     * Deserializes and handles received messages.
     *
     * See class comment.
     *
     * @param messageType  The type of the received message.
     * @param messageBuffer A buffer containing a serialized message, starting
     *                      at index 0.
     * @param messageSize  The length in bytes of the message in messageBuffer.
     */
    void processReceivedMessage(Uint8 messageType, Uint8* messageBuffer,
                                std::size_t messageSize);

    /**
     * Returns the latest tick that we've received an update message for.
     */
    Uint32 getLastReceivedTick();

    /**
     * See extension member comment.
     */
    void setExtension(std::unique_ptr<IMessageProcessorExtension> inExtension);

private:
    //-------------------------------------------------------------------------
    // Handlers for messages relevant to the network layer.
    //-------------------------------------------------------------------------
    /** Pushes ExplicitConfirmation event. */
    void handleExplicitConfirmation(Uint8* messageBuffer,
                                    std::size_t messageSize);

    /** Pushes ConnectionResponse event. */
    void handleConnectionResponse(Uint8* messageBuffer,
                                  std::size_t messageSize);

    /** Pushes PlayerMovementUpdate and MovementUpdate events. */
    void handleMovementUpdate(Uint8* messageBuffer, std::size_t messageSize);

    /** Pushes ComponentUpdate event. */
    void handleComponentUpdate(Uint8* messageBuffer, std::size_t messageSize);
    //-------------------------------------------------------------------------

    /** The dispatcher for network events. Used to send events to the
        subscribed queues. */
    EventDispatcher& networkEventDispatcher;

    /** The entity that this client is controlling. Used when we receive client
        entity updates, to tell if they contain data relevant to the player
        entity. */
    entt::entity playerEntity;

    /** The latest tick that we've received a message or confirmation for. */
    std::atomic<Uint32> lastReceivedTick;

    /** If non-nullptr, contains the project's message processing extension
        functions.
        Allows the project to provide message processing code and have it be
        called at the appropriate time. */
    std::unique_ptr<IMessageProcessorExtension> extension;
};

} // End namespace Client
} // End namespace AM
