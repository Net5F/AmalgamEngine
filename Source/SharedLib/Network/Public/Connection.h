#pragma once

#include "TlsTransport.h"
#include "Log.h"
#include "asio/error.hpp"
#include "asio/steady_timer.hpp"
#include "asio/system_error.hpp"
#include <chrono>
#include <exception>
#include <functional>
#include <memory>
#include <utility>

namespace AM
{

/**
 * Base class for our connection objects.
 *
 * Manages the async TLS handshake and maintains the connection state.
 */
class Connection : public std::enable_shared_from_this<Connection>
{
private:
    enum class State { NotStarted, Handshaking, Ready, Closing, Closed };

public:
    using ReadyCallback = std::function<void(Connection&)>;
    using DisconnectCallback = std::function<void(const asio::error_code&)>;

    Connection(asio::ip::tcp::socket socket, asio::ssl::context& sslContext,
               const TlsTransport::ClassConfig& config)
    : tlsTransport{
          std::make_shared<TlsTransport>(std::move(socket), sslContext, config)}
    , readyCallback{}
    , disconnectCallback{}
    , state{State::NotStarted}
    , closeTimer{tlsTransport->getExecutor()}
    {
    }

    virtual ~Connection()
    {
        if (tlsTransport) {
            tlsTransport->close();
        }
    }

    /**
     * Starts this connection as a TLS server.
     *
     * Note: The callbacks must not strongly capture this Connection. Connection
     *       stores these callbacks, so a strong capture would create an
     *       ownership cycle. Capture a weak_ptr<Connection> or ConnectionHandle
     *       instead.
     */
    void startServer(ReadyCallback inReadyCallback,
                     DisconnectCallback inDisconnectCallback)
    {
        start(TlsTransport::HandshakeType::server, std::move(inReadyCallback),
              std::move(inDisconnectCallback));
    }

    /**
     * Starts this connection as a TLS client.
     *
     * Note: The callbacks must not strongly capture this Connection. Connection
     *       stores these callbacks, so a strong capture would create an
     *       ownership cycle. Capture a weak_ptr<Connection> or ConnectionHandle
     *       instead.
     */
    void startClient(ReadyCallback inReadyCallback,
                     DisconnectCallback inDisconnectCallback)
    {
        start(TlsTransport::HandshakeType::client, std::move(inReadyCallback),
              std::move(inDisconnectCallback));
    }

    /**
     * Disconnects this peer and invokes the registered callback once.
     */
    void disconnect(const asio::error_code& error)
    {
        if (state == State::Closed) {
            return;
        }

        state = State::Closed;
        readyCallback = {};
        // Note: We move the callback into a local so it can't destroy itself
        //       while running by destroying this connection object.
        DisconnectCallback callback{std::move(disconnectCallback)};
        disconnectCallback = {};

        cancelCloseTimer();
        tlsTransport->close();

        if (callback) {
            callback(error);
        }
    }

    /**
     * Closes without invoking the disconnect callback.
     */
    void close()
    {
        if (state == State::Closed) {
            return;
        }

        state = State::Closed;
        cancelCloseTimer();
        tlsTransport->close();
    }

    /**
     * Stops accepting sends. Once all queued sends have been written, closes
     * our side of the connection and waits for the peer to close theirs.
     * When that happens (or closeTimeoutS passes), disconnects and invokes
     * the registered callback once.
     *
     * Use this instead of disconnect() when the peer should receive our
     * final messages (e.g. a message telling it why it's being
     * disconnected).
     *
     * Note: While closing, derived classes should keep reading (and
     *       discarding) data, so that the peer closing is detected.
     */
    void closeAfterWrites(double closeTimeoutS)
    {
        if (state == State::Closing) {
            return;
        }
        if (state != State::Ready) {
            // There's nothing to flush, just disconnect.
            disconnect(asio::error::make_error_code(asio::error::shut_down));
            return;
        }

        state = State::Closing;

        // If the peer doesn't close in time, give up on a clean close.
        try {
            closeTimer.expires_after(
                std::chrono::duration_cast<asio::steady_timer::duration>(
                    std::chrono::duration<double>(closeTimeoutS)));

            std::weak_ptr<Connection> weakSelf{shared_from_this()};
            closeTimer.async_wait([weakSelf](const asio::error_code& error) {
                if (error == asio::error::operation_aborted) {
                    return;
                }
                if (std::shared_ptr<Connection> self{weakSelf.lock()}) {
                    self->disconnect(
                        asio::error::make_error_code(asio::error::timed_out));
                }
            });
        } catch (const asio::system_error& e) {
            disconnect(e.code());
            return;
        }

        tlsTransport->closeAfterWrites();
    }

    /**
     * Returns true if the transport is open, else false.
     */
    bool isOpen() const { return (tlsTransport && tlsTransport->isOpen()); }

    /**
     * Returns true if the TLS handshake and peer validation have completed.
     */
    bool isReady() const { return state == State::Ready; }

    /**
     * Returns true if closeAfterWrites() was called and we're waiting for
     * the connection to finish closing.
     */
    bool isClosing() const { return state == State::Closing; }

    /**
     * Returns the underlying OpenSSL handle. Useful for inspecting the
     * peer's certificate once the handshake has completed.
     */
    TlsTransport::NativeHandle nativeHandle()
    {
        return tlsTransport->nativeHandle();
    }

protected:
    /**
     * Tells our derived class to begin reading messages.
     */
    virtual void startReadLoop() = 0;

    /** Note: This is a shared_ptr so it can safely post async events that
              reference itself. */
    std::shared_ptr<TlsTransport> tlsTransport;

private:
    void start(TlsTransport::HandshakeType handshakeType,
               ReadyCallback inReadyCallback,
               DisconnectCallback inDisconnectCallback)
    {
        if (state != State::NotStarted) {
            LOG_INFO("Connection cannot be started from its current state.");
            return;
        }

        std::shared_ptr<Connection> self{shared_from_this()};
        state = State::Handshaking;
        readyCallback = std::move(inReadyCallback);
        disconnectCallback = std::move(inDisconnectCallback);

        std::weak_ptr<Connection> weakSelf{self};
        tlsTransport->setFailureCallback(
            [weakSelf](const asio::error_code& error) {
                if (std::shared_ptr<Connection> lockedSelf{weakSelf.lock()}) {
                    lockedSelf->disconnect(error);
                }
            });

        if (!tlsTransport->handshake(handshakeType, [weakSelf]() {
                if (std::shared_ptr<Connection> lockedSelf{weakSelf.lock()}) {
                    lockedSelf->handleHandshakeComplete();
                }
            })) {
            disconnect(
                asio::error::make_error_code(asio::error::not_connected));
        }
    }

    void handleHandshakeComplete()
    {
        if (state != State::Handshaking) {
            LOG_INFO("Called handleHandshakeComplete while not handshaking.");
            return;
        }

        // Note: We move the callback into a local so it can't destroy itself
        //       while running by calling disconnect().
        ReadyCallback callback{std::move(readyCallback)};
        readyCallback = {};

        state = State::Ready;

        // Call the ready callback.
        if (callback) {
            callback(*this);
        }

        // The callback may have closed or disconnected this connection.
        if (state != State::Ready) {
            return;
        }

        startReadLoop();
    }

    void cancelCloseTimer()
    {
        try {
            closeTimer.cancel();
        } catch (const asio::system_error& e) {
            LOG_INFO("Failed to cancel close timer: %s",
                     e.code().message().c_str());
        }
    }

    // Note: We don't expect these to throw. It's up to the caller to ensure
    //       that.
    ReadyCallback readyCallback;
    DisconnectCallback disconnectCallback;

    State state;

    /** Bounds how long closeAfterWrites() waits for the peer to close. */
    asio::steady_timer closeTimer;
};

} // namespace AM
