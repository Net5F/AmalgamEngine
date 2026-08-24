#pragma once

#include "TlsTransport.h"
#include "Log.h"
#include "asio/error.hpp"
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
    enum class State { NotStarted, Handshaking, Ready, Closed };

public:
    using ReadyCallback = std::function<bool(Connection&)>;
    using DisconnectCallback = std::function<void(const asio::error_code&)>;

    Connection(asio::ip::tcp::socket socket, asio::ssl::context& sslContext,
               const TlsTransport::ClassConfig& config)
    : tlsTransport{std::make_shared<TlsTransport>(
          std::move(socket), sslContext, config)}
    , readyCallback{}
    , disconnectCallback{}
    , state{State::NotStarted}
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
                     DisconnectCallback inDisconnectCallback, double timeoutS)
    {
        start(TlsTransport::HandshakeType::server, std::move(inReadyCallback),
              std::move(inDisconnectCallback), timeoutS);
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
                     DisconnectCallback inDisconnectCallback, double timeoutS)
    {
        start(TlsTransport::HandshakeType::client, std::move(inReadyCallback),
              std::move(inDisconnectCallback), timeoutS);
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
        tlsTransport->close();
    }

    /**
     * Returns true if the transport is open, else false.
     */
    bool isOpen() const { return (tlsTransport && tlsTransport->isOpen()); }

    /**
     * Returns true if the TLS handshake and peer validation have completed.
     */
    bool isReady() const { return state == State::Ready; }

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
               DisconnectCallback inDisconnectCallback, double timeoutS)
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

        if (!tlsTransport->handshake(handshakeType, timeoutS, [weakSelf]() {
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

        // Call the ready callback, giving our parent the opportunity to reject
        // this connection if desired.
        if (callback && !callback(*this)) {
            disconnect(
                asio::error::make_error_code(asio::error::access_denied));
            return;
        }

        // The callback may have closed or disconnected this connection.
        if (state != State::Handshaking) {
            return;
        }

        state = State::Ready;
        startReadLoop();
    }

    // Note: We don't expect these to throw. It's up to the caller to ensure that.
    ReadyCallback readyCallback;
    DisconnectCallback disconnectCallback;

    State state;
};

} // namespace AM
