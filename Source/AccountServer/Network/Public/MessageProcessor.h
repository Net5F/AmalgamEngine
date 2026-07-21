#pragma once

#include "NetworkID.h"
#include "asio/thread_pool.hpp"
#include "asio/io_context.hpp"
#include <SDL3/SDL_stdinc.h>
#include <functional>
#include <span>

namespace AM
{
struct AccountRegisterRequest;

namespace AccountServer
{

/**
 * Processes received messages.
 *
 * Unlike Client/Server, which pass messages down to the sim layer, 
 * AccountServer handles messages directly in the network thread. As such, 
 * these functions are actual message handlers, instead of just dispatchers.
 *
 * To add a message:
 *   1. Add #include "MyNewMessage.h" to MessageProcessor.cpp.
 *   2. Add a case to the switch statement in processReceivedMessage().
 *   3. Add a forward declaration and handleMessage(NetworkID, MyNewMessage) 
 *      function.
 */
class MessageProcessor
{
public:
    using SendCallback = std::function<void(NetworkID, BinaryBufferSharedPtr)>;

    MessageProcessor(asio::io_context& inNetworkIoContext,
                     asio::thread_pool& inDatabasePool,
                     SendCallback sendCallback);

    /**
     * Deserializes and handles received messages.
     *
     * @param netID The network ID of the client that the message came from.
     * @param messageType The type of the received message.
     * @param messageBuffer A buffer containing a serialized message, starting
     * at index 0.
     */
    void processReceivedMessage(NetworkID netID, Uint8 messageType,
                                std::span<const Uint8> messageBuffer);

private:
    //-------------------------------------------------------------------------
    // Handlers
    //-------------------------------------------------------------------------
    void handleMessage(NetworkID netID, const AccountRegisterRequest& message);
    
    //-------------------------------------------------------------------------
    // Helpers 
    //-------------------------------------------------------------------------
    template<typename Message>
    void dispatchMessage(NetworkID netID, std::span<const Uint8> messageBuffer);

    template<typename Message>
    BinaryBufferSharedPtr serializeMessage(const Message& message);

    asio::io_context& networkIoContext;
    asio::thread_pool& databasePool;

    /** Used to send messages through ClientManager.
        Needed since ClientManager owns the NetworkID -> Client map. */
    SendCallback sendCallback;
};

} // End namespace AccountServer
} // End namespace AM
