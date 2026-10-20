#pragma once

#include "Connection.h"
#include "WorldBatchComposer.h"
#include "BinaryBuffer.h"
#include "asio/steady_timer.hpp"
#include <functional>

namespace AM
{
namespace Client
{

/**
 * A connection object for the Client <-> WorldServer protocol.
 *
 * On top of the base Connection class, this class adds batch receiving and
 * framed message sending.
 */
class WorldConnection : public Connection
{
public:
    using BatchCallback
        = std::function<void(const WorldBatchComposer::ReceivedBatch&)>;

    struct ClassConfig {
        TlsTransport::ClassConfig tlsTransportConfig{};
        /** If we receive nothing for this long, we disconnect. */
        double idleTimeoutS{10};
    };

    WorldConnection(asio::ip::tcp::socket socket,
                    asio::ssl::context& sslContext, const ClassConfig& config);

    /**
     * Sets the callback invoked when a complete batch is received.
     *
     * Note: The callback must not strongly capture this WorldConnection.
     *       WorldConnection stores this callback, so a strong capture would
     *       create an ownership cycle.
     */
    void setBatchCallback(BatchCallback inBatchCallback);

    /**
     * Async sends a message that was framed by WorldMessageFramer.
     */
    bool sendFramed(BinaryBufferSharedPtr message);

private:
    void startReadLoop() override;

    /**
     * Async reads the next part of the batch.
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

    /** Used to detect when the server stops sending us data.
        Note: Rather than restarting this timer for every batch, we track
              lastReceiveTime and check it when the timer expires. */
    asio::steady_timer idleTimer;

    /** The last time that we received a complete batch. */
    asio::steady_timer::time_point lastReceiveTime;

    WorldBatchComposer batchComposer;

    // Note: We don't expect this to throw. It's up to the caller to ensure
    //       that.
    BatchCallback batchCallback;
};

} // namespace Client
} // namespace AM
