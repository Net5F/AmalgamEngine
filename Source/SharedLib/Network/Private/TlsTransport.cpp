#include "TlsTransport.h"
#include "Log.h"
#include "asio/error.hpp"
#include "asio/read.hpp"
#include "asio/socket_base.hpp"
#include "asio/system_error.hpp"
#include "asio/write.hpp"
#include <openssl/ssl.h>
#include <utility>

namespace AM
{
TlsTransport::TlsTransport(asio::ip::tcp::socket socket,
                           asio::ssl::context& sslContext,
                           const ClassConfig& config)
try
: stream{std::move(socket), sslContext}
, maxQueuedWriteBytes{config.maxQueuedWriteBytes}
, maxQueuedMessages{config.maxQueuedMessages}
, handshakeTimer{stream.get_executor()}
, writeQueue{}
, queuedWriteBytes{0}
, failureCallback{}
, closed{false}
{
    if (SSL_set_min_proto_version(stream.native_handle(), TLS1_3_VERSION)
        != 1) {
        LOG_FATAL("Failed to require TLS 1.3.");
    }

    asio::error_code error{};
    stream.lowest_layer().set_option(asio::socket_base::keep_alive{true},
                                     error);
    if (error) {
        LOG_FATAL("Failed to enable TCP keep-alive: %s",
                  error.message().c_str());
    }
}
catch (const asio::system_error& e) {
    LOG_FATAL("Failed to construct TLS transport: %s", e.what());
}

TlsTransport::~TlsTransport()
{
    close();
}

void TlsTransport::setFailureCallback(FailureCallback inFailureCallback)
{
    failureCallback = std::move(inFailureCallback);
}

bool TlsTransport::handshake(HandshakeType type, double timeoutS,
                             SuccessCallback successCallback)
{
    if (closed) {
        return false;
    }
    std::shared_ptr<TlsTransport> self{shared_from_this()};
    try {
        handshakeTimer.expires_after(
            std::chrono::duration_cast<asio::steady_timer::duration>(
                std::chrono::duration<double>(timeoutS)));
        handshakeTimer.async_wait([self](const asio::error_code& error) {
            if (!error) {
                self->reportFailure(
                    asio::error::make_error_code(asio::error::timed_out));
            }
        });

        stream.async_handshake(
            type, [self, successCallback = std::move(successCallback)](
                const asio::error_code& error) mutable {
                if (!self->cancelHandshakeTimer()) {
                    self->reportFailure(
                        asio::error::make_error_code(asio::error::fault));
                    return;
                }
                if (self->closed) {
                    return;
                }
                if (error) {
                    self->reportFailure(error);
                    return;
                }

                if (successCallback) {
                    successCallback();
                }
            });
    } catch (const asio::system_error& e) {
        reportFailure(e.code());
        return false;
    }
    return true;
}

bool TlsTransport::asyncReadExactly(asio::mutable_buffer buffer,
                                    ReadCallback readCallback)
{
    if (closed) {
        return false;
    }

    std::shared_ptr<TlsTransport> self{shared_from_this()};
    try {
        asio::async_read(
            stream, buffer,
            [self, readCallback = std::move(readCallback)](
                const asio::error_code& error, std::size_t bytesRead) mutable {
                if (self->closed) {
                    return;
                }
                if (error) {
                    self->reportFailure(error);
                    return;
                }

                if (readCallback) {
                    readCallback(bytesRead);
                }
            });
    } catch (const asio::system_error& e) {
        reportFailure(e.code());
        return false;
    }
    return true;
}

bool TlsTransport::asyncWrite(BinaryBufferSharedPtr buffer)
{
    if (closed || !buffer) {
        return false;
    }
    if (buffer->empty()) {
        return true;
    }

    if ((writeQueue.size() >= maxQueuedMessages)
        || (buffer->size() > maxQueuedWriteBytes)
        || (queuedWriteBytes > (maxQueuedWriteBytes - buffer->size()))) {
        reportFailure(
            asio::error::make_error_code(asio::error::no_buffer_space));
        return false;
    }

    bool writeInProgress{!writeQueue.empty()};
    queuedWriteBytes += buffer->size();
    writeQueue.push_back(std::move(buffer));
    if (!writeInProgress) {
        return sendNext();
    }

    return true;
}

void TlsTransport::close()
{
    if (closed) {
        return;
    }

    closed = true;
    failureCallback = {};
    cancelHandshakeTimer();
    closeSocket();
    writeQueue.clear();
    queuedWriteBytes = 0;
}

bool TlsTransport::isOpen() const
{
    return !closed && stream.lowest_layer().is_open();
}

TlsTransport::NativeHandle TlsTransport::nativeHandle()
{
    return stream.native_handle();
}

asio::any_io_executor TlsTransport::getExecutor()
{
    return stream.get_executor();
}

void TlsTransport::reportFailure(const asio::error_code& error)
{
    if (closed) {
        return;
    }

    closed = true;
    cancelHandshakeTimer();
    closeSocket();
    writeQueue.clear();
    queuedWriteBytes = 0;

    if (failureCallback) {
        failureCallback(error);
    }
    failureCallback = {};
}

bool TlsTransport::sendNext()
{
    if (closed || writeQueue.empty()) {
        return false;
    }

    BinaryBufferSharedPtr activeBuffer{writeQueue.front()};
    std::shared_ptr<TlsTransport> self{shared_from_this()};
    try {
        asio::async_write(
            stream, asio::buffer(*activeBuffer),
            [self, activeBuffer](const asio::error_code& error, std::size_t) {
                if (self->closed) {
                    return;
                }
                if (error) {
                    self->reportFailure(error);
                    return;
                }

                self->queuedWriteBytes -= activeBuffer->size();
                self->writeQueue.pop_front();
                self->sendNext();
            });
    } catch (const asio::system_error& e) {
        reportFailure(e.code());
        return false;
    }

    return true;
}

bool TlsTransport::cancelHandshakeTimer()
{
    try {
        handshakeTimer.cancel();
    } catch (const asio::system_error& e) {
        LOG_INFO("Failed to cancel TLS handshake timer: %s",
                 e.code().message().c_str());
        return false;
    }

    return true;
}

void TlsTransport::closeSocket()
{
    asio::error_code ignoredError{};
    stream.lowest_layer().cancel(ignoredError);
    stream.lowest_layer().shutdown(asio::ip::tcp::socket::shutdown_both,
                                   ignoredError);
    stream.lowest_layer().close(ignoredError);
}

} // namespace AM
