#pragma once

#include "AccountDefs.h"
#include "AccountServiceMessageType.h"
#include "ConsumeServiceTicketResponse.h"
#include "SimpleConnection.h"
#include "SimpleMessageFramer.h"
#include "asio/io_context.hpp"
#include "asio/ip/tcp.hpp"
#include "asio/ssl/context.hpp"
#include "asio/steady_timer.hpp"
#include <array>
#include <cstddef>
#include <functional>
#include <memory>
#include <span>
#include <string>
#include <unordered_map>

namespace AM
{
namespace WorldServer
{

/**
 * Manages communication with the AccountServer.
 *
 * We use this connection to validate the tickets that clients present to us
 * when logging in.
 *
 * Peers are authenticated through mutual TLS: the given sslContext must
 * present our certificate, and must verify the AccountServer's certificate
 * against its pin.
 *
 * We hold this connection open for as long as we're running. Once start() is
 * called, we connect and send periodic heartbeats to keep the connection
 * alive. If the connection fails or is lost, we reconnect with an increasing
 * delay. Requests that are made while we're disconnected fail immediately.
 *
 * Note: Public functions are thread-safe. All other work happens on the
 *       network IO thread.
 */
class AccountServiceEndpoint
{
public:
    using ServiceTicket = std::array<Uint8, SERVICE_TICKET_BYTES>;

    /**
     * Called with the AccountServer's response to a ticket request.
     *
     * If the AccountServer couldn't be reached or the connection was lost
     * before it responded, response.result will be InternalError.
     *
     * Note: This is called on the network IO thread. Callers should forward
     *       the result to their own thread rather than doing work here.
     */
    using ConsumeServiceTicketCallback
        = std::function<void(const ConsumeServiceTicketResponse&)>;

    AccountServiceEndpoint(asio::io_context& inIoContext,
                           asio::ssl::context& inSSLContext);

    ~AccountServiceEndpoint();

    /**
     * Connects to the AccountServer. Must be called once before any requests
     * will succeed.
     */
    void start();

    /**
     * Asks the AccountServer to validate and consume the given ticket.
     *
     * The ticket is only accepted if it was issued for the World Server.
     *
     * If we aren't currently connected to the AccountServer, the request
     * fails immediately.
     *
     * The callback is always called exactly once, unless this endpoint is
     * destroyed first.
     */
    void consumeServiceTicket(const ServiceTicket& ticket,
                              ConsumeServiceTicketCallback callback);

private:
    using Connection = SimpleConnection<AccountServiceMessageType>;

    enum class ConnectionState { Disconnected, Connecting, Connected };

    // Note: If a project ever cares to configure any of these, they can be
    //       moved into Config.h.
    static constexpr double CONNECTING_TIMEOUT_S{5};
    static constexpr double PARTIAL_RECEIVE_TIMEOUT_S{10};
    /** How often we send a heartbeat. Must be well below the AccountServer's
        SERVICE_IDLE_TIMEOUT_S, so it doesn't time us out. */
    static constexpr double HEARTBEAT_INTERVAL_S{15};
    /** How long we'll go without receiving anything before assuming the
        AccountServer is gone. The AccountServer responds to each heartbeat,
        so this is 3 missed heartbeat responses. */
    static constexpr double IDLE_TIMEOUT_S{HEARTBEAT_INTERVAL_S * 3};
    /** How long we wait before reconnecting after a connection failure or
        disconnect. Doubles after each failed attempt, up to the max. */
    static constexpr double RECONNECT_MIN_DELAY_S{1};
    static constexpr double RECONNECT_MAX_DELAY_S{30};
    static constexpr std::size_t MAX_QUEUED_WRITE_BYTES{64000};
    static constexpr std::size_t MAX_QUEUED_WRITES{1000};
    static constexpr Uint16 MAX_READ_PAYLOAD_SIZE{500};
    static constexpr Uint16 MAX_WRITE_PAYLOAD_SIZE{500};
    /** The max number of requests that may be awaiting a response. */
    static constexpr std::size_t MAX_PENDING_REQUESTS{1000};

    void consumeServiceTicketOnIOThread(const ServiceTicket& ticket,
                                        ConsumeServiceTicketCallback callback);

    /** Connection logic */
    void connectOnIOThread();
    void beginConnect(std::string serverIP, unsigned int serverPort);
    void handleConnectionFailure(std::size_t currentAttempt,
                                 const asio::error_code& error);
    void disconnectWithError(const asio::error_code& error);

    /**
     * Waits for reconnectDelayS, then attempts to connect. Increases the
     * delay for next time.
     */
    void scheduleReconnect();

    /**
     * Waits for HEARTBEAT_INTERVAL_S, then sends a heartbeat and schedules
     * the next one.
     */
    void scheduleHeartbeat(std::size_t currentAttempt);

    /** Cancels the given timer, logging any errors. */
    void cancelTimer(asio::steady_timer& timer);

    /**
     * Sends the given request. If the send fails, fails the request.
     */
    void sendRequest(Uint32 requestID, BinaryBufferSharedPtr framedMessage);

    /**
     * Calls the given request's callback with the given response, and
     * removes the request.
     */
    void completeRequest(Uint32 requestID,
                         const ConsumeServiceTicketResponse& response);

    /**
     * Fails the given request with an InternalError result.
     */
    void failRequest(Uint32 requestID);

    /**
     * Fails all pending requests with an InternalError result.
     */
    void failAllRequests();

    /** Event handlers. */
    void onSocketConnected(std::shared_ptr<asio::ip::tcp::socket> socket,
                           std::size_t currentAttempt,
                           const asio::error_code& error);
    void onConnectionReady(std::size_t currentAttempt);
    void onConnectionDisconnected(std::size_t currentAttempt,
                                  const asio::error_code& error);
    void onMessageReceived(AccountServiceMessageType messageType,
                           std::span<const Uint8> messageBuffer);

    void handleMessage(const ConsumeServiceTicketResponse& response);

    asio::io_context& ioContext;
    asio::ssl::context& sslContext;

    /** Used to time out connection attempts. */
    asio::steady_timer connectionTimer;
    /** Used to wait between reconnect attempts. */
    asio::steady_timer reconnectTimer;
    /** Used to send periodic heartbeats. */
    asio::steady_timer heartbeatTimer;

    std::shared_ptr<asio::ip::tcp::socket> connectingSocket;
    std::shared_ptr<Connection> connection;

    SimpleMessageFramer<AccountServiceMessageType> messageFramer;

    /** Maps request IDs to the callback that's waiting on a response. */
    std::unordered_map<Uint32, ConsumeServiceTicketCallback> pendingRequests;

    /** The ID to give the next request. */
    Uint32 nextRequestID;

    /** How long to wait before the next reconnect attempt. */
    double reconnectDelayS;

    /** True if start() has been called. */
    bool started;

    ConnectionState connectionState;
    /** Used to track which connection attempt we're on, so late async
        callbacks for already-canceled attempts don't get processed. */
    std::size_t connectionAttempt;
};

} // namespace WorldServer
} // namespace AM
