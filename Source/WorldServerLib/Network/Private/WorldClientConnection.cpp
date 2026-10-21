#include "WorldClientConnection.h"
#include "Log.h"
#include "asio/error.hpp"
#include "asio/system_error.hpp"
#include <chrono>
#include <memory>
#include <utility>

namespace AM
{
namespace WorldServer
{

WorldClientConnection::WorldClientConnection(asio::ip::tcp::socket socket,
                                             asio::ssl::context& sslContext,
                                             const ClassConfig& config)
: Connection{std::move(socket), sslContext, config.tlsTransportConfig}
, idleTimeout{std::chrono::duration_cast<asio::steady_timer::duration>(
      std::chrono::duration<double>(config.idleTimeoutS))}
, idleTimer{tlsTransport->getExecutor()}
, lastReceiveTime{}
, authTimer{tlsTransport->getExecutor()}
, messageComposer{config.maxReadPayloadSize}
, messageCallback{}
{
}

void WorldClientConnection::setMessageCallback(
    MessageCallback inMessageCallback)
{
    messageCallback = std::move(inMessageCallback);
}

void WorldClientConnection::setMaxReadPayloadSize(
    std::size_t maxReadPayloadSize)
{
    messageComposer.setMaxPayloadSize(maxReadPayloadSize);
}

bool WorldClientConnection::sendFramed(BinaryBufferSharedPtr batch)
{
    if (!isReady()) {
        return false;
    }

    return tlsTransport->asyncWrite(std::move(batch));
}

void WorldClientConnection::startAuthTimeout(double timeoutS)
{
    try {
        authTimer.expires_after(
            std::chrono::duration_cast<asio::steady_timer::duration>(
                std::chrono::duration<double>(timeoutS)));

        std::weak_ptr<WorldClientConnection> weakSelf{
            std::static_pointer_cast<WorldClientConnection>(
                shared_from_this())};
        authTimer.async_wait([weakSelf](const asio::error_code& error) {
            if (error == asio::error::operation_aborted) {
                return;
            }
            if (std::shared_ptr<WorldClientConnection> self{weakSelf.lock()}) {
                self->disconnect(error ? error
                                       : asio::error::make_error_code(
                                             asio::error::timed_out));
            }
        });
    } catch (const asio::system_error& e) {
        disconnect(e.code());
    }
}

void WorldClientConnection::cancelAuthTimeout()
{
    try {
        authTimer.cancel();
    } catch (const asio::system_error& e) {
        disconnect(e.code());
    }
}

void WorldClientConnection::startReadLoop()
{
    lastReceiveTime = asio::steady_timer::clock_type::now();
    if (!startIdleTimer(lastReceiveTime + idleTimeout)) {
        return;
    }

    readNext();
}

bool WorldClientConnection::canRead() const
{
    return isReady() || isClosing();
}

void WorldClientConnection::readNext()
{
    if (!canRead()) {
        return;
    }

    std::shared_ptr<WorldClientConnection> self{
        std::static_pointer_cast<WorldClientConnection>(shared_from_this())};
    if (!tlsTransport->asyncReadExactly(
            messageComposer.nextReadBuffer(),
            [self](std::size_t) { self->handleRead(); })) {
        disconnect(asio::error::make_error_code(asio::error::not_connected));
    }
}

void WorldClientConnection::handleRead()
{
    if (!canRead()) {
        return;
    }

    // Pass the bytes to the composer. If a message hasn't been completed,
    // start the next async read.
    WorldClientMessageComposer::ComposeResult composeResult{
        messageComposer.onBytesRead()};
    if (composeResult.error) {
        disconnect(composeResult.error);
        return;
    }
    if (!composeResult.messageComplete) {
        readNext();
        return;
    }

    // If we're closing, discard the message.
    if (isClosing()) {
        readNext();
        return;
    }

    lastReceiveTime = asio::steady_timer::clock_type::now();

    // A message was completed. Pass it to the registered callback.
    if (!messageCallback) {
        disconnect(asio::error::make_error_code(asio::error::invalid_argument));
        return;
    }
    messageCallback(composeResult.message);
    if (!canRead()) {
        return;
    }

    readNext();
}

bool WorldClientConnection::startIdleTimer(
    asio::steady_timer::time_point expiryTime)
{
    try {
        idleTimer.expires_at(expiryTime);

        std::weak_ptr<WorldClientConnection> weakSelf{
            std::static_pointer_cast<WorldClientConnection>(
                shared_from_this())};
        idleTimer.async_wait([weakSelf](const asio::error_code& error) {
            std::shared_ptr<WorldClientConnection> self{weakSelf.lock()};
            if (!self || !self->isReady()
                || (error == asio::error::operation_aborted)) {
                // Already disconnected, closing, or timer canceled.
                return;
            }
            if (error) {
                // Error while connected: disconnect.
                self->disconnect(error);
                return;
            }

            self->handleIdleTimerExpired();
        });
    } catch (const asio::system_error& e) {
        disconnect(e.code());
        return false;
    }

    return true;
}

void WorldClientConnection::handleIdleTimerExpired()
{
    // If we've received something since the timer was started, wait for the
    // rest of the timeout.
    asio::steady_timer::time_point expiryTime{lastReceiveTime + idleTimeout};
    if (asio::steady_timer::clock_type::now() < expiryTime) {
        startIdleTimer(expiryTime);
        return;
    }

    disconnect(asio::error::make_error_code(asio::error::timed_out));
}

} // End namespace WorldServer
} // End namespace AM
