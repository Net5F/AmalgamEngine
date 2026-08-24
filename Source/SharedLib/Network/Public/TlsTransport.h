#pragma once

#include "BinaryBuffer.h"
#include "asio/any_io_executor.hpp"
#include "asio/buffer.hpp"
#include "asio/error_code.hpp"
#include "asio/ip/tcp.hpp"
#include "asio/ssl/context.hpp"
#include "asio/ssl/stream.hpp"
#include "asio/steady_timer.hpp"
#include <chrono>
#include <cstddef>
#include <deque>
#include <functional>
#include <memory>

namespace AM
{
/**
 * Asynchronous TLS transport shared by endpoint connection types.
 *
 * All functions must be called from the stream executor. The transport owns
 * its write queue so callers may enqueue messages while a write is active.
 */
class TlsTransport : public std::enable_shared_from_this<TlsTransport>
{
public:
    using HandshakeType = asio::ssl::stream_base::handshake_type;
    using SuccessCallback = std::function<void()>;
    using ReadCallback = std::function<void(std::size_t)>;
    using FailureCallback = std::function<void(const asio::error_code&)>;
    using NativeHandle
        = asio::ssl::stream<asio::ip::tcp::socket>::native_handle_type;

    struct ClassConfig {
        /** How many bytes we allow to be queued for writing at once. */
        std::size_t maxQueuedWriteBytes{4000};
        /** How many messages we allow to be queued for writing at once. */
        std::size_t maxQueuedMessages{10};
    };

    TlsTransport(asio::ip::tcp::socket socket, asio::ssl::context& sslContext,
                 const ClassConfig& config);

    virtual ~TlsTransport();

    TlsTransport(const TlsTransport&) = delete;
    TlsTransport& operator=(const TlsTransport&) = delete;
    TlsTransport(TlsTransport&&) = delete;
    TlsTransport& operator=(TlsTransport&&) = delete;

    /**
     * Sets the callback invoked once when a transport operation fails.
     *
     * Note: The callback must not strongly capture the Connection that owns
     *       this transport. Connection owns TlsTransport, and TlsTransport
     *       stores this callback, so a strong capture would create an
     *       ownership cycle. Capture a weak_ptr<Connection> or
     *       ConnectionHandle instead.
     */
    void setFailureCallback(FailureCallback inFailureCallback);

    /**
     * Performs a TLS handshake, bounded by the given timeout.
     */
    bool handshake(HandshakeType type, double timeoutS,
                   SuccessCallback successCallback);

    /**
     * Reads exactly buffer.size() bytes.
     */
    bool asyncReadExactly(asio::mutable_buffer buffer,
                          ReadCallback readCallback);

    /**
     * Queues a complete frame for ordered delivery.
     */
    bool asyncWrite(BinaryBufferSharedPtr buffer);

    /**
     * Cancels outstanding operations and closes the underlying socket.
     */
    void close();

    /**
     * Returns true if this transport is open and ready for use.
     */
    bool isOpen() const;

    NativeHandle nativeHandle();

    asio::any_io_executor getExecutor();

protected:
    /** Closes the transport and reports a failure exactly once. */
    void reportFailure(const asio::error_code& error);

private:
    bool sendNext();
    bool cancelHandshakeTimer();
    void closeSocket();

    asio::ssl::stream<asio::ip::tcp::socket> stream;

    // See comments in Config above.
    std::size_t maxQueuedWriteBytes{};
    std::size_t maxQueuedMessages{};

    /** The timer we use to detect when a handshake has taken too long. */
    asio::steady_timer handshakeTimer;

    std::deque<BinaryBufferSharedPtr> writeQueue;
    std::size_t queuedWriteBytes;

    FailureCallback failureCallback;
    bool closed;
};

} // namespace AM
