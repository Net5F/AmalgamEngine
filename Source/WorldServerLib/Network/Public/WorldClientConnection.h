#pragma once

#include "Connection.h"
#include "WorldClientMessageComposer.h"
#include "BinaryBuffer.h"
#include "asio/steady_timer.hpp"
#include <functional>

namespace AM
{
namespace WorldServer
{

/**
 * A connection object for the WorldServer <-> Client protocol.
 *
 * On top of the base Connection class, this class adds message receiving,
 * batch sending, and the timeouts that clients are held to.
 */
class WorldClientConnection : public Connection
{
public:
    using MessageCallback = std::function<void(
        const WorldClientMessageComposer::ReceivedMessage&)>;

    struct ClassConfig {
        TlsTransport::ClassConfig tlsTransportConfig{};
        /** If we receive no complete messages for this long, we
            disconnect. */
        double idleTimeoutS{4};
        /** The max size, in bytes, for an incoming message payload. */
        std::size_t maxReadPayloadSize{0};
    };

    WorldClientConnection(asio::ip::tcp::socket socket,
                          asio::ssl::context& sslContext,
                          const ClassConfig& config);

    /**
     * Sets the callback invoked when a complete message is received.
     *
     * Note: The callback must not strongly capture this connection.
     *       WorldClientConnection stores this callback, so a strong capture
     *       would create an ownership cycle.
     */
    void setMessageCallback(MessageCallback inMessageCallback);

    /**
     * Sets the max size, in bytes, for an incoming message payload.
     */
    void setMaxReadPayloadSize(std::size_t maxReadPayloadSize);

    /**
     * Async sends a batch that was framed by WorldBatchFramer.
     */
    bool sendFramed(BinaryBufferSharedPtr batch);

    /**
     * Starts a timer. If cancelAuthTimeout() isn't called before it expires,
     * disconnects.
     */
    void startAuthTimeout(double timeoutS);

    /**
     * Cancels the timer started by startAuthTimeout().
     */
    void cancelAuthTimeout();

private:
    void startReadLoop() override;

    /**
     * Returns true if we should keep reading.
     * Note: While closing, we keep reading (and discarding) so we can detect
     *       the peer closing its side.
     */
    bool canRead() const;

    /**
     * Async reads the next part of the message.
     */
    void readNext();

    /**
     * Handles a completed async read.
     */
    void handleRead();

    /**
     * Starts the idle timer, set to expire at the given time.
     */
    bool startIdleTimer(asio::steady_timer::time_point expiryTime);

    /**
     * If we haven't received anything within the idle timeout, disconnects.
     * Otherwise, restarts the timer for the remaining time.
     */
    void handleIdleTimerExpired();

    asio::steady_timer::duration idleTimeout;

    /** Used to detect when the client stops sending us data.
        Note: Rather than restarting this timer for every message, we track
              lastReceiveTime and check it when the timer expires. */
    asio::steady_timer idleTimer;

    /** The last time that we received a complete message. */
    asio::steady_timer::time_point lastReceiveTime;

    /** Used to drop clients that don't authenticate in time. */
    asio::steady_timer authTimer;

    WorldClientMessageComposer messageComposer;

    // Note: We don't expect this to throw. It's up to the caller to ensure
    //       that.
    MessageCallback messageCallback;
};

} // End namespace WorldServer
} // End namespace AM
