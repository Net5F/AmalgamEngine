#pragma once

#include "NetworkID.h"
#include "NetworkDefs.h"
#include "BinaryBuffer.h"
#include "asio/ip/tcp.hpp"
#include "asio/steady_timer.hpp"
#include <array>
#include <memory>
#include <queue>

namespace AM
{
namespace AccountServer
{
/**
 * Represents a single networked client.
 *
 * Manages sending/receiving messages, and connection state.
 */
class Client : public std::enable_shared_from_this<Client>
{
public:
    using MessageCallback
        = std::function<void(NetworkID, Uint8, std::span<const Uint8>)>;
    using DisconnectCallback = std::function<void(NetworkID, asio::error_code)>;

    Client(NetworkID inNetID, asio::ip::tcp::socket inSocket,
           MessageCallback inMessageCallback,
           DisconnectCallback inDisconnectCallback);

    /**
     * Starts this client's message receiving loop.
     *
     * Runs asynchronously through asio.
     */
    void startReceiveLoop();

    /**
     * Queues the given message for sending.
     */
    void send(BinaryBufferSharedPtr message);

private:
    /**
     * Async message send.
     */
    void sendNextMessage();

    /**
     * Async header read.
     */
    void readHeader();

    /**
     * Async payload read.
     */
    void readPayload(Uint8 messageType, Uint16 payloadSize);

    /**
     * Starts or resets the message timer.
     */
    void startMessageTimer();

    /**
     * Marks this client as disconnected and closes the socket connection.
     */
    void disconnect(const asio::error_code& error);

    /** Our Network-given ID. */
    NetworkID netID;

    /** Our connection and interface to the client. */
    asio::ip::tcp::socket socket;

    /** Used to process received messages. */
    MessageCallback messageCallback;
    DisconnectCallback disconnectCallback;

    /** Tracks whether we've disconnected. */
    bool isConnected;

    /** Receive buffers. */
    std::array<Uint8, MESSAGE_HEADER_SIZE> headerBuffer;
    BinaryBuffer payloadBuffer;

    /** Send buffers. Must be kept alive until they're processed by 
        async_write. */
    std::queue<BinaryBufferSharedPtr> sendQueue;

    /** Tracks how long it's been since we've received a message from this
        client. */
    asio::steady_timer messageTimer;
};

} // End namespace AccountServer
} // End namespace AM
