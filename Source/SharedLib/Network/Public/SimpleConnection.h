#pragma once

#include "Connection.h"
#include "SimpleMessageFramer.h"
#include "SimpleMessageComposer.h"
#include "BinaryBuffer.h"
#include "ByteTools.h"
#include "Log.h"
#include "asio/buffer.hpp"
#include "asio/error.hpp"
#include "asio/steady_timer.hpp"
#include "asio/system_error.hpp"
#include <chrono>
#include <cstddef>
#include <exception>
#include <functional>
#include <memory>
#include <span>
#include <stdexcept>
#include <utility>

namespace AM
{

/**
 * A connection object for our simple "type + size, no batching" protocol. This
 * protocol is used by most endpoints.
 *
 * On top of the base Connection class, this class adds messaging sending/
 * receiving.
 */
template<typename MessageType>
class SimpleConnection : public Connection
{
public:
    using MessageCallback
        = std::function<void(MessageType, std::span<const Uint8>)>;

    struct ClassConfig {
        TlsTransport::ClassConfig tlsTransportConfig{};
        /** How long after receiving the start of a message to wait for the
            rest. */
        double partialReceiveTimeoutS{10};
        /** How long to allow a connection to be idle. */
        double idleTimeoutS{60};
        /** The max size, in bytes, for an incoming message payload. */
        Uint16 maxReadPayloadSize{500};
        /** The max size, in bytes, for an outgoing message payload. */
        Uint16 maxWritePayloadSize{500};
    };

    SimpleConnection(asio::ip::tcp::socket socket,
                     asio::ssl::context& sslContext, const ClassConfig& config)
    : Connection{std::move(socket), sslContext, config.tlsTransportConfig}
    , partialReceiveTimeoutS{config.partialReceiveTimeoutS}
    , idleTimeoutS{config.idleTimeoutS}
    , receiveTimer{tlsTransport->getExecutor()}
    , messageFramer{config.maxWritePayloadSize}
    , messageComposer{config.maxReadPayloadSize}
    , messageCallback{}
    {
        std::size_t maxWriteFrameSize{MESSAGE_HEADER_SIZE
                                      + config.maxWritePayloadSize};
        if (config.tlsTransportConfig.maxQueuedWriteBytes < maxWriteFrameSize) {
            LOG_FATAL("TLS write queue byte limit is smaller than the max "
                      "write frame size.");
        }
    }

    /**
     * Sets the callback invoked when a complete message is received.
     *
     * Note: The callback must not strongly capture this SimpleConnection.
     *       SimpleConnection stores this callback, so a strong capture would
     *       create an ownership cycle. Capture a weak_ptr<SimpleConnection> or
     *       ConnectionHandle instead.
     */
    void setMessageCallback(MessageCallback inMessageCallback)
    {
        messageCallback = std::move(inMessageCallback);
    }

    /**
     * Frames and async sends the given message.
     */
    template<typename Message>
    bool send(const Message& message)
    {
        if (!isReady()) {
            return false;
        }

        return tlsTransport->asyncWrite(messageFramer.frameMessage(message));
    }

    /**
     * Sends a message that has already been framed for this connection's
     * protocol.
     *
     * To frame a message for use in this function, construct a framer:
     * SimpleMessageFramer<MyMessageType> messageFramer{MAX_SIZE};
     * BinaryBufferSharedPtr framedMessage{messageFramer.frameMessage(message)};
     */
    bool sendFramed(BinaryBufferSharedPtr message)
    {
        if (!isReady()) {
            return false;
        }

        return tlsTransport->asyncWrite(std::move(message));
    }

private:
    using Self = SimpleConnection<MessageType>;

    void startReadLoop() override { readNext(); }

    /**
     * Async reads the next amount of data.
     */
    void readNext()
    {
        if (!isReady()) {
            return;
        }

        // If we're waiting for a new frame, start the idle timer.
        // Note: The composer reads a frame's first byte separately, so we can
        //       distinguish an idle connection from a partially received
        //       frame. This lets us detect slowloris-style attacks.
        if (!messageComposer.isFrameInProgress()) {
            if (!startReceiveTimer(idleTimeoutS)) {
                return;
            }
        }

        std::shared_ptr<Self> self{
            std::static_pointer_cast<Self>(shared_from_this())};
        if (!tlsTransport->asyncReadExactly(
                messageComposer.getNextReadBuffer(),
                [self](std::size_t) { self->handleRead(); })) {
            cancelReceiveTimer();
            disconnect(
                asio::error::make_error_code(asio::error::not_connected));
        }
    }

    /**
     * Handles a completed async read.
     */
    void handleRead()
    {
        if (!isReady()) {
            return;
        }

        // Tell the composer to process the read bytes (they were written
        // directly into its buffer).
        auto composeResult{messageComposer.processReadBytes()};
        if (composeResult.error) {
            cancelReceiveTimer();
            disconnect(composeResult.error);
            return;
        }

        // If a frame was started, switch from the idle timer to the partial
        // receive timer.
        if (composeResult.frameStarted) {
            if (!cancelReceiveTimer()
                || !startReceiveTimer(partialReceiveTimeoutS)) {
                return;
            }
        }

        // If a message hasn't been completed, start the next async read.
        if (!composeResult.frameComplete) {
            readNext();
            return;
        }

        // A message was completed. Process it, then wipe its payload since
        // it may hold secrets.
        std::span<Uint8> payload{composeResult.message.payload};
        bool keepReading{processMessage(composeResult.message.type, payload)};
        ByteTools::wipe(payload.data(), payload.size());
        if (keepReading) {
            readNext();
        }
    }

    /**
     * Passes a completed message to the registered callback.
     *
     * @return true if we should keep reading, else false.
     */
    bool processMessage(MessageType type, std::span<const Uint8> payload)
    {
        if (!cancelReceiveTimer()) {
            return false;
        }

        if (!messageCallback) {
            disconnect(
                asio::error::make_error_code(asio::error::invalid_argument));
            return false;
        }
        messageCallback(type, payload);

        return isReady();
    }

    /**
     * Starts a timer. Upon expiry, disconnects this connection.
     */
    bool startReceiveTimer(double timeoutS)
    {
        try {
            receiveTimer.expires_after(
                std::chrono::duration_cast<asio::steady_timer::duration>(
                    std::chrono::duration<double>(timeoutS)));

            std::weak_ptr<Self> weakSelf{
                std::static_pointer_cast<Self>(shared_from_this())};
            receiveTimer.async_wait([weakSelf](const asio::error_code& error) {
                std::shared_ptr<Self> self{weakSelf.lock()};
                if (!self || !self->isReady()
                    || (error == asio::error::operation_aborted)) {
                    // Already disconnected or timer canceled.
                    return;
                }
                if (error) {
                    // Error while connected: disconnect.
                    self->disconnect(error);
                    return;
                }

                // Timer expired, disconnect with timeout.
                self->disconnect(
                    asio::error::make_error_code(asio::error::timed_out));
            });
        } catch (const asio::system_error& e) {
            disconnect(e.code());
            return false;
        }

        return true;
    }

    /**
     * Cancels a running receiveTimer.
     */
    bool cancelReceiveTimer()
    {
        try {
            receiveTimer.cancel();
        } catch (const asio::system_error& e) {
            disconnect(e.code());
            return false;
        }

        return true;
    }

    // See comments in Config above.
    double partialReceiveTimeoutS;
    double idleTimeoutS;

    /** The timer we use to detect when clients are fully idle, or when they
        have sent a partial message but aren't completing it quickly enough. */
    asio::steady_timer receiveTimer;

    SimpleMessageFramer<MessageType> messageFramer;
    /** Owns our receive buffers. On destruction, wipes them. */
    SimpleMessageComposer<MessageType> messageComposer;

    // Note: We don't expect this to throw. It's up to the caller to ensure
    // that.
    MessageCallback messageCallback;
};

} // namespace AM
