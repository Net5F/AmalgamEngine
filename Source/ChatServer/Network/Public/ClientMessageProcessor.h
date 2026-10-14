#pragma once

#include "ChatClientMessageType.h"
#include "ConnectionHandle.h"
#include "SimpleMessageFramer.h"
#include "asio/error_code.hpp"
#include <SDL3/SDL_stdinc.h>
#include <functional>
#include <span>

namespace AM
{
struct ChatHeartbeat;

namespace ChatServer
{

/**
 * Processes ChatClientMessageType messages.
 *
 * Like AccountServer, ChatServer handles messages directly in the network
 * thread. As such, these functions are actual message handlers, instead of just
 * dispatchers.
 *
 * To add a message:
 *   1. Add #include "MyNewMessage.h" to ClientMessageProcessor.cpp.
 *   2. Add a case to the switch statement in processReceivedMessage().
 *   3. Add a forward declaration and a matching handleMessage() function.
 */
class ClientMessageProcessor
{
public:
    using SendCallback
        = std::function<void(ConnectionHandle, BinaryBufferSharedPtr)>;
    using DisconnectCallback
        = std::function<void(ConnectionHandle, const asio::error_code&)>;

    ClientMessageProcessor(SendCallback inSendCallback,
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
                                ChatClientMessageType messageType,
                                std::span<const Uint8> messageBuffer);

private:
    //-------------------------------------------------------------------------
    // Handlers
    //-------------------------------------------------------------------------
    void handleMessage(ConnectionHandle handle, const ChatHeartbeat& message);

    //-------------------------------------------------------------------------
    // Helpers
    //-------------------------------------------------------------------------
    template<typename Message>
    void handleMessage(ConnectionHandle handle,
                       std::span<const Uint8> messageBuffer);

    /** Used to send messages through the owning endpoint. */
    SendCallback sendCallback;

    /** Used to disconnect clients that send invalid messages. */
    DisconnectCallback disconnectCallback;

    /** Used to serialize and frame our outgoing messages. */
    SimpleMessageFramer<ChatClientMessageType> messageFramer;
};

} // End namespace ChatServer
} // End namespace AM
