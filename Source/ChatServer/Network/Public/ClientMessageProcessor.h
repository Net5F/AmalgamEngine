#pragma once

#include "ChatClientMessageType.h"
#include "ConnectionHandle.h"
#include "SimpleMessageFramer.h"
#include "ConsumeServiceTicketResponse.h"
#include "asio/error_code.hpp"
#include <SDL3/SDL_stdinc.h>
#include <chrono>
#include <functional>
#include <span>
#include <string>
#include <unordered_map>

namespace AM
{
struct ChatConnectionRequest;
struct ChatSendRequest;
struct ChatHeartbeat;
class AccountServiceEndpoint;

namespace ChatServer
{

/**
 * Processes ChatClientMessageType messages.
 *
 * Like AccountServer, ChatServer handles messages directly in the network
 * thread. As such, these functions are actual message handlers, instead of just
 * dispatchers.
 *
 * Also tracks each client's session. A client must send a
 * ChatConnectionRequest with a valid Chat Server ticket before it can send or
 * receive chat. If its ticket is rejected, it isn't allowed to send any more
 * messages, and will be disconnected if it tries.
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

    ClientMessageProcessor(AccountServiceEndpoint& inAccountEndpoint,
                           SendCallback inSendCallback,
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

    /**
     * Cleans up the given client's session. Must be called when a client
     * disconnects.
     */
    void onClientDisconnected(ConnectionHandle handle);

private:
    /** A client's chat session. */
    struct ClientSession {
        enum class State {
            /** We're waiting for the client to send a ChatConnectionRequest. */
            AwaitingTicket,
            /** We're waiting for the AccountServer to validate the client's
                ticket. */
            ValidatingTicket,
            /** The client is logged in and can send and receive chat. */
            Authenticated,
            /** The client's ticket was rejected. */
            Rejected
        };
        State state{State::AwaitingTicket};

        /** If authenticated, the ID of the client's account. */
        Sint64 accountID{0};

        /** If authenticated, the client's username. */
        std::string username{};

        /** How many messages the client can currently send before being rate
            limited. */
        double availableMessages{0};

        /** The last time that availableMessages was refilled. */
        std::chrono::steady_clock::time_point lastRefillTime{};
    };

    //-------------------------------------------------------------------------
    // Handlers
    //-------------------------------------------------------------------------
    void handleMessage(ConnectionHandle handle, ClientSession& session,
                       const ChatConnectionRequest& message);

    void handleMessage(ConnectionHandle handle, ClientSession& session,
                       const ChatSendRequest& message);

    void handleMessage(ConnectionHandle handle, ClientSession& session,
                       const ChatHeartbeat& message);

    //-------------------------------------------------------------------------
    // Helpers
    //-------------------------------------------------------------------------
    /**
     * Handles the AccountServer's response to the given client's ticket.
     */
    void handleTicketResponse(ConnectionHandle handle,
                              const ConsumeServiceTicketResponse& response);

    /**
     * Refills the given session's available messages, then tries to use one.
     * @return true if a message was available, else false (the client is
     *         sending too quickly).
     */
    bool tryUseMessage(ClientSession& session);

    /**
     * Sends the given message to every authenticated client.
     */
    void broadcast(BinaryBufferSharedPtr message);

    template<typename Message>
    void handleMessage(ConnectionHandle handle, ClientSession& session,
                       std::span<const Uint8> messageBuffer);

    /** Used to validate client tickets. */
    AccountServiceEndpoint& accountEndpoint;

    /** Used to send messages through the owning endpoint. */
    SendCallback sendCallback;

    /** Used to disconnect clients that send invalid messages. */
    DisconnectCallback disconnectCallback;

    /** Used to serialize and frame our outgoing messages. */
    SimpleMessageFramer<ChatClientMessageType> messageFramer;

    /** The session of each connected client. */
    std::unordered_map<ConnectionHandle, ClientSession> sessions;

    /** Maps account IDs to the client that's authenticated as that account. */
    std::unordered_map<Sint64, ConnectionHandle> accountClients;
};

} // End namespace ChatServer
} // End namespace AM
