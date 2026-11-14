#pragma once

#include "AccountDefs.h"
#include "AccountServiceMessageType.h"
#include "ConsumeServiceTicketResponse.h"
#include "SaveAccountDataResponse.h"
#include "AccountData.h"
#include "SimpleConnection.h"
#include "SimpleMessageFramer.h"
#include "asio/io_context.hpp"
#include "asio/ip/tcp.hpp"
#include "asio/ssl/context.hpp"
#include "asio/steady_timer.hpp"
#include <array>
#include <cstddef>
#include <functional>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <unordered_map>
#include <variant>

namespace AM
{

/**
 * Used by our internal services (world server, chat server) to communicate
 * with AccountServer.
 *
 * Services use this connection to validate the tickets that clients present
 * to them when logging in. The world server also uses it to save its client
 * entities' AccountData.
 *
 * Peers are authenticated through mutual TLS: the given sslContext must
 * present our certificate, and must verify AccountServer's certificate
 * against its pin.
 *
 * We hold this connection open for as long as we're running. Once start() is
 * called, we connect and send periodic heartbeats to keep the connection
 * alive. If the connection fails or is lost, we reconnect with an increasing
 * delay. Requests that are made while we're disconnected fail immediately.
 *
 * Note: Public functions are thread-safe. All other work happens on the
 *       network IO thread.
 *
 * Note: This file is in a "Shared" subdirectory because AccountServer has its
 *       own AccountServiceEndpoint.h (the other side of this connection), and
 *       it links SharedLib. A flat include would be ambiguous there, so this
 *       one must be included as "Shared/AccountServiceEndpoint.h".
 */
class AccountServiceEndpoint
{
public:
    /**
     * Called with AccountServer's response to a ticket request.
     *
     * If AccountServer couldn't be reached or the connection was lost
     * before it responded, response.result will be InternalError.
     *
     * Note: This is called on the network IO thread. Callers should forward
     *       the result to their own thread rather than doing work here.
     */
    using ConsumeServiceTicketCallback
        = std::function<void(const ConsumeServiceTicketResponse&)>;

    /**
     * Called with AccountServer's response to a save request.
     *
     * If AccountServer couldn't be reached or the connection was lost
     * before it responded, response.result will be InternalError.
     *
     * Note: This is called on the network IO thread. Callers should forward
     *       the result to their own thread rather than doing work here.
     */
    using SaveAccountDataCallback
        = std::function<void(const SaveAccountDataResponse&)>;

    /**
     * @param inSSLContext Must present the service's certificate, and verify
     *                     AccountServer's certificate against its pin.
     * @param inServerIP AccountServer's IP address.
     * @param inServerPort AccountServer's service port.
     */
    AccountServiceEndpoint(asio::io_context& inIoContext,
                           asio::ssl::context& inSSLContext,
                           const std::string& inServerIP,
                           unsigned int inServerPort);

    ~AccountServiceEndpoint();

    /**
     * Connects to the account server. Must be called once before any requests
     * will succeed.
     */
    void start();

    /**
     * Asks the account server to validate and consume the given ticket.
     *
     * The ticket is only accepted if it was issued for the service that owns
     * this client.
     *
     * If includeAccountData is true, the response will include the account's
     * saved AccountData. Only the world server may set it.
     *
     * If we aren't currently connected to the account server, the request
     * fails immediately.
     *
     * The callback is always called exactly once, unless this endpoint is
     * destroyed first.
     */
    void consumeServiceTicket(const ServiceTicket& ticket,
                              bool includeAccountData,
                              ConsumeServiceTicketCallback callback);

    /**
     * Asks the account server to save the given account's AccountData,
     * overwriting any existing data. Only the world server may call this.
     *
     * The data must be within its size limits (see
     * AccountData::isWithinSizeLimits()), or the request fails immediately.
     *
     * If we aren't currently connected to the account server, the request
     * fails immediately.
     *
     * The callback is always called exactly once, unless this endpoint is
     * destroyed first.
     */
    void saveAccountData(Sint64 accountID, AccountData accountData,
                         SaveAccountDataCallback callback);

private:
    using Connection = SimpleConnection<AccountServiceMessageType>;

    /** The callback of a request that's awaiting a response. */
    using PendingCallback
        = std::variant<ConsumeServiceTicketCallback, SaveAccountDataCallback>;

    enum class ConnectionState { Disconnected, Connecting, Connected };

    // Note: If a project ever cares to configure any of these, they can be
    //       made constructor parameters.
    static constexpr double CONNECTING_TIMEOUT_S{5};
    static constexpr double PARTIAL_RECEIVE_TIMEOUT_S{10};
    /** How often we send a heartbeat. Must be well below AccountServer's
        SERVICE_IDLE_TIMEOUT_S, so it doesn't time us out. */
    static constexpr double HEARTBEAT_INTERVAL_S{15};
    /** How long we'll go without receiving anything before assuming
        AccountServer is gone. AccountServer responds to each heartbeat,
        so this is 3 missed heartbeat responses. */
    static constexpr double IDLE_TIMEOUT_S{HEARTBEAT_INTERVAL_S * 3};
    /** How long we wait before reconnecting after a connection failure or
        disconnect. Doubles after each failed attempt, up to the max. */
    static constexpr double RECONNECT_MIN_DELAY_S{1};
    static constexpr double RECONNECT_MAX_DELAY_S{30};
    /** Large enough to hold many in-flight AccountData saves. */
    static constexpr std::size_t MAX_QUEUED_WRITE_BYTES{2'000'000};
    static constexpr std::size_t MAX_QUEUED_WRITES{1000};
    /** Large enough to hold a message carrying AccountData. */
    static constexpr Uint16 MAX_READ_PAYLOAD_SIZE{
        std::numeric_limits<Uint16>::max()};
    static constexpr Uint16 MAX_WRITE_PAYLOAD_SIZE{
        std::numeric_limits<Uint16>::max()};
    /** The max number of requests that may be awaiting a response. */
    static constexpr std::size_t MAX_PENDING_REQUESTS{1000};

    void consumeServiceTicketOnIOThread(const ServiceTicket& ticket,
                                        bool includeAccountData,
                                        ConsumeServiceTicketCallback callback);
    void saveAccountDataOnIOThread(Sint64 accountID, AccountData& accountData,
                                   SaveAccountDataCallback callback);

    /** Connection logic */
    void connectOnIOThread();
    void beginConnect();
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

    /** Request logic */
    /**
     * If we're able to send a request, tracks the given callback and returns
     * its request ID. Otherwise, fails the callback and returns null.
     */
    std::optional<Uint32> beginRequest(PendingCallback& callback);

    /**
     * Sends the given request. If the send fails, fails the request.
     */
    void sendRequest(Uint32 requestID, BinaryBufferSharedPtr framedMessage);

    /**
     * Calls the given request's callback with the given response, and
     * removes the request.
     *
     * If the request's callback doesn't match the response type, fails the
     * request instead.
     */
    template<typename Response>
    void completeRequest(Uint32 requestID, const Response& response);

    /**
     * Calls the given callback with an InternalError result.
     */
    void failCallback(Uint32 requestID, PendingCallback& callback);

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

    /**
     * Deserializes a response of the given type and completes its request.
     */
    template<typename Response>
    void handleResponse(std::span<const Uint8> messageBuffer);

    asio::io_context& ioContext;
    asio::ssl::context& sslContext;

    /** AccountServer's address. */
    std::string serverIP;
    unsigned int serverPort;

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
    std::unordered_map<Uint32, PendingCallback> pendingRequests;

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

} // namespace AM
