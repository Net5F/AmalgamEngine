#include "WorldConnection.h"
#include "NetworkDefs.h"
#include "Log.h"
#include "asio/error.hpp"
#include "asio/system_error.hpp"
#include <chrono>
#include <memory>
#include <utility>

namespace AM
{
namespace Client
{

WorldConnection::WorldConnection(asio::ip::tcp::socket socket,
                                 asio::ssl::context& sslContext,
                                 const ClassConfig& config)
: Connection{std::move(socket), sslContext, config.tlsTransportConfig}
, idleTimeout{std::chrono::duration_cast<asio::steady_timer::duration>(
      std::chrono::duration<double>(config.idleTimeoutS))}
, idleTimer{tlsTransport->getExecutor()}
, lastReceiveTime{}
, batchComposer{}
, batchCallback{}
{
    if (config.tlsTransportConfig.maxQueuedWriteBytes
        < CLIENT_MAX_MESSAGE_SIZE) {
        LOG_FATAL("TLS write queue byte limit is smaller than the max client "
                  "message size.");
    }
}

void WorldConnection::setBatchCallback(BatchCallback inBatchCallback)
{
    batchCallback = std::move(inBatchCallback);
}

bool WorldConnection::sendFramed(BinaryBufferSharedPtr message)
{
    if (!isReady()) {
        return false;
    }

    return tlsTransport->asyncWrite(std::move(message));
}

void WorldConnection::startReadLoop()
{
    lastReceiveTime = asio::steady_timer::clock_type::now();
    if (!startIdleTimer(lastReceiveTime + idleTimeout)) {
        return;
    }

    readNext();
}

void WorldConnection::readNext()
{
    if (!isReady()) {
        return;
    }

    std::shared_ptr<WorldConnection> self{
        std::static_pointer_cast<WorldConnection>(shared_from_this())};
    if (!tlsTransport->asyncReadExactly(
            batchComposer.nextReadBuffer(),
            [self](std::size_t) { self->handleRead(); })) {
        disconnect(asio::error::make_error_code(asio::error::not_connected));
    }
}

void WorldConnection::handleRead()
{
    if (!isReady()) {
        return;
    }

    // Pass the bytes to the composer. If a batch hasn't been completed, start
    // the next async read.
    WorldBatchComposer::ComposeResult composeResult{
        batchComposer.onBytesRead()};
    if (composeResult.error) {
        disconnect(composeResult.error);
        return;
    }
    if (!composeResult.batchComplete) {
        readNext();
        return;
    }

    lastReceiveTime = asio::steady_timer::clock_type::now();

    // A batch was completed. Pass it to the registered callback.
    if (!batchCallback) {
        disconnect(asio::error::make_error_code(asio::error::invalid_argument));
        return;
    }
    batchCallback(composeResult.batch);
    if (!isReady()) {
        return;
    }

    readNext();
}

bool WorldConnection::startIdleTimer(asio::steady_timer::time_point expiryTime)
{
    try {
        idleTimer.expires_at(expiryTime);

        std::weak_ptr<WorldConnection> weakSelf{
            std::static_pointer_cast<WorldConnection>(shared_from_this())};
        idleTimer.async_wait([weakSelf](const asio::error_code& error) {
            std::shared_ptr<WorldConnection> self{weakSelf.lock()};
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

            self->handleIdleTimerExpired();
        });
    } catch (const asio::system_error& e) {
        disconnect(e.code());
        return false;
    }

    return true;
}

void WorldConnection::handleIdleTimerExpired()
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

} // namespace Client
} // namespace AM
