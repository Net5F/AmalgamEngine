#pragma once

#include "ChatServiceMessageType.h"
#include "ConnectionHandle.h"
#include "SimpleMessageFramer.h"
#include "asio/error_code.hpp"
#include <SDL3/SDL_stdinc.h>
#include <functional>
#include <span>

namespace AM
{
struct ChatServiceHeartbeat;

namespace ChatServer
{

/**
 * Processes ChatServiceMessageType messages.
 *
 * These messages are received from trusted services (world server).
 * ChatServiceEndpoint authenticates the services before any of their messages
 * reach this class.
 *
 * See ClientMessageProcessor for info on adding a message.
 */
class ServiceMessageProcessor
{
public:
    using SendCallback
        = std::function<void(ConnectionHandle, BinaryBufferSharedPtr)>;
    using DisconnectCallback
        = std::function<void(ConnectionHandle, const asio::error_code&)>;

    ServiceMessageProcessor(SendCallback inSendCallback,
                            DisconnectCallback inDisconnectCallback);

    /**
     * Deserializes and handles received messages.
     *
     * @param handle The connection that the message came from.
     * @param messageType The type of the received message.
     * @param messageBuffer A buffer containing a serialized message, starting
     * at index 0.
     */
    void processReceivedMessage(ConnectionHandle handle,
                                ChatServiceMessageType messageType,
                                std::span<const Uint8> messageBuffer);

private:
    //-------------------------------------------------------------------------
    // Handlers
    //-------------------------------------------------------------------------
    void handleMessage(ConnectionHandle handle,
                       const ChatServiceHeartbeat& message);

    //-------------------------------------------------------------------------
    // Helpers
    //-------------------------------------------------------------------------
    template<typename Message>
    void handleMessage(ConnectionHandle handle,
                       std::span<const Uint8> messageBuffer);

    /** Used to send messages through the owning endpoint. */
    SendCallback sendCallback;

    /** Used to disconnect services that send invalid messages. */
    DisconnectCallback disconnectCallback;

    /** Used to serialize and frame our outgoing messages. */
    SimpleMessageFramer<ChatServiceMessageType> messageFramer;
};

} // End namespace ChatServer
} // End namespace AM
