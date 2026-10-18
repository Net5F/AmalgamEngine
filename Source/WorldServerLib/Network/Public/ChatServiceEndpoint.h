#pragma once

#include "ChatServiceMessageType.h"
#include "PlayerPositionsUpdate.h"
#include "PlayersLeftWorld.h"
#include "SimpleConnection.h"
#include "Log.h"
#include "asio/io_context.hpp"
#include "asio/ip/tcp.hpp"
#include "asio/post.hpp"
#include "asio/ssl/context.hpp"
#include "asio/steady_timer.hpp"
#include <SDL3/SDL_stdinc.h>
#include <algorithm>
#include <cstddef>
#include <memory>
#include <span>
#include <string>
#include <utility>

namespace AM
{
namespace WorldServer
{

/**
 * Used to communicate with ChatServer.
 *
 * We use this connection to tell the chat server where client entities are,
 * so it can route local chat.
 *
 * Peers are authenticated through mutual TLS: the given sslContext must
 * present our certificate, and must verify ChatServer's certificate
 * against its pin.
 *
 * We hold this connection open for as long as we're running. Once start() is
 * called, we connect and send periodic heartbeats to keep the connection
 * alive. If the connection fails or is lost, we reconnect with an increasing
 * delay. Messages that are sent while we're disconnected are dropped.
 *
 * Note: The chat server forgets all positions when we connect or disconnect.
 *       ChatPositionSyncSystem sends every position on each update, so it
 *       doesn't need to do anything special after we reconnect.
 *
 * Note: Public functions are thread-safe. All other work happens on the
 *       network IO thread.
 */
class ChatServiceEndpoint
{
public:
    /**
     * @param inSSLContext Must present our certificate, and verify
     *                     ChatServer's certificate against its pin.
     * @param inServerIP ChatServer's IP address.
     * @param inServerPort ChatServer's service port.
     */
    ChatServiceEndpoint(asio::io_context& inIoContext,
                        asio::ssl::context& inSSLContext,
                        const std::string& inServerIP,
                        unsigned int inServerPort);

    ~ChatServiceEndpoint();

    /**
     * Connects to the chat server. Must be called once before any messages
     * will be sent.
     */
    void start();

    /**
     * Sends the given message to the chat server.
     *
     * If we aren't currently connected, the message is dropped.
     */
    template<typename Message>
    void send(Message message)
    {
        asio::post(ioContext, [this, message = std::move(message)]() {
            sendOnIOThread(message);
        });
    }

private:
    using Connection = SimpleConnection<ChatServiceMessageType>;

    enum class ConnectionState { Disconnected, Connecting, Connected };

    // Note: If a project ever cares to configure any of these, they can be
    //       made constructor parameters.
    static constexpr double CONNECTING_TIMEOUT_S{5};
    static constexpr double PARTIAL_RECEIVE_TIMEOUT_S{10};
    /** How often we send a heartbeat. Must be well below ChatServer's
        SERVICE_IDLE_TIMEOUT_S, so it doesn't time us out. */
    static constexpr double HEARTBEAT_INTERVAL_S{15};
    /** How long we'll go without receiving anything before assuming
        ChatServer is gone. ChatServer responds to each heartbeat, so this
        is 3 missed heartbeat responses. */
    static constexpr double IDLE_TIMEOUT_S{HEARTBEAT_INTERVAL_S * 3};
    /** How long we wait before reconnecting after a connection failure or
        disconnect. Doubles after each failed attempt, up to the max. */
    static constexpr double RECONNECT_MIN_DELAY_S{1};
    static constexpr double RECONNECT_MAX_DELAY_S{30};
    /** Every client entity's position is sent at once. This fits ~50 full
        PlayerPositionsUpdates (~50,000 client entities). */
    static constexpr std::size_t MAX_QUEUED_WRITE_BYTES{1000000};
    static constexpr std::size_t MAX_QUEUED_WRITES{1000};
    static constexpr Uint16 MAX_READ_PAYLOAD_SIZE{500};
    /** Note: Both messages static_assert that they fit in a Uint16. */
    static constexpr Uint16 MAX_WRITE_PAYLOAD_SIZE{
        static_cast<Uint16>(std::max(PlayerPositionsUpdate::MAX_SERIALIZED_SIZE,
                                     PlayersLeftWorld::MAX_SERIALIZED_SIZE))};

    template<typename Message>
    void sendOnIOThread(const Message& message)
    {
        if (connectionState != ConnectionState::Connected) {
            return;
        }

        // Note: We hold a local reference, since a failed send will
        //       disconnect and reset our member. In that case, we'll
        //       reconnect and the next update will resend everything.
        if (std::shared_ptr<Connection> activeConnection{connection}) {
            if (!(activeConnection->send(message))) {
                LOG_INFO("Failed to send a ChatServer message.");
            }
        }
    }

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

    /** Event handlers. */
    void onSocketConnected(std::shared_ptr<asio::ip::tcp::socket> socket,
                           std::size_t currentAttempt,
                           const asio::error_code& error);
    void onConnectionReady(std::size_t currentAttempt);
    void onConnectionDisconnected(std::size_t currentAttempt,
                                  const asio::error_code& error);
    void onMessageReceived(ChatServiceMessageType messageType,
                           std::span<const Uint8> messageBuffer);

    /** Cancels the given timer, logging any errors. */
    void cancelTimer(asio::steady_timer& timer);

    /** Returns the given number of seconds as a timer duration. */
    asio::steady_timer::duration toTimerDuration(double seconds);

    asio::io_context& ioContext;
    asio::ssl::context& sslContext;

    /** ChatServer's address. */
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
