#pragma once

#include "Log.h"
#include "asio/error.hpp"
#include "asio/error_code.hpp"
#include "asio/io_context.hpp"
#include "asio/ip/tcp.hpp"
#include "asio/socket_base.hpp"
#include <exception>
#include <functional>
#include <memory>
#include <utility>

// TODO: Rename this file to Acceptor.h when we delete the current one.

namespace AM
{
/**
 * Reusable asynchronous TCP listener for accepting connections.
 *
 * The parent Endpoint class supplies connection construction and registration
 * callbacks. TLS handshaking remains the responsibility of Connection.
 */
template<typename ConnectionType>
class Acceptor
{
public:
    using ConnectionFactory
        = std::function<std::shared_ptr<ConnectionType>(asio::ip::tcp::socket)>;
    using AcceptCallback = std::function<void(std::shared_ptr<ConnectionType>)>;
    using ErrorCallback = std::function<void(const asio::error_code&)>;

    Acceptor(asio::io_context& ioContext, asio::ip::tcp::endpoint endpoint,
             ConnectionFactory connectionFactory, AcceptCallback acceptCallback,
             ErrorCallback errorCallback)
    : acceptLoop{std::make_shared<AcceptLoop>(
          ioContext, std::move(endpoint), std::move(connectionFactory),
          std::move(acceptCallback), std::move(errorCallback))}
    {
    }

    Acceptor(Acceptor&& other) noexcept
    : acceptLoop{std::move(other.acceptLoop)}
    {
    }

    Acceptor(const Acceptor&) = delete;
    Acceptor& operator=(const Acceptor&) = delete;

    ~Acceptor() { stop(); }

    Acceptor& operator=(Acceptor&& other) noexcept
    {
        if (this != &other) {
            stop();
            acceptLoop = std::move(other.acceptLoop);
        }
        return *this;
    }

    bool start() { return acceptLoop && acceptLoop->start(); }

    void stop()
    {
        if (acceptLoop) {
            acceptLoop->stop();
        }
    }

    bool isRunning() const { return acceptLoop && acceptLoop->running; }

private:
    /**
     * Manages the async TCP connection accept loop.
     *
     * We use a separate class so it can be owned as a shared_ptr by the async
     * context, making it safe to call, even during shutdown.
     */
    struct AcceptLoop : public std::enable_shared_from_this<AcceptLoop> {
        AcceptLoop(asio::io_context& ioContext,
                   asio::ip::tcp::endpoint inEndpoint,
                   ConnectionFactory inConnectionFactory,
                   AcceptCallback inAcceptCallback,
                   ErrorCallback inErrorCallback)
        : acceptor{ioContext}
        , endpoint{std::move(inEndpoint)}
        , connectionFactory{std::move(inConnectionFactory)}
        , acceptCallback{std::move(inAcceptCallback)}
        , errorCallback{std::move(inErrorCallback)}
        , running{false}
        {
        }

        /**
         * Starts the accept loop.
         */
        bool start()
        {
            if (running) {
                return true;
            }
            if (!connectionFactory || !acceptCallback) {
                reportError(asio::error::make_error_code(
                    asio::error::invalid_argument));
                return false;
            }

            asio::error_code error{};
            acceptor.open(endpoint.protocol(), error);
            if (!error) {
                acceptor.set_option(asio::socket_base::reuse_address{true},
                                    error);
            }
            if (!error) {
                acceptor.bind(endpoint, error);
            }
            if (!error) {
                acceptor.listen(asio::socket_base::max_listen_connections,
                                error);
            }
            if (error) {
                asio::error_code ignoredError{};
                acceptor.close(ignoredError);
                reportError(error);
                return false;
            }

            running = true;
            acceptNext();
            return true;
        }

        /**
         * Stops the accept loop.
         */
        void stop()
        {
            if (!running && !acceptor.is_open()) {
                return;
            }

            running = false;
            asio::error_code ignoredError{};
            acceptor.cancel(ignoredError);
            acceptor.close(ignoredError);
        }

        /**
         * Stages an async handler for the next accept event.
         */
        void acceptNext()
        {
            if (!running) {
                return;
            }

            std::shared_ptr<AcceptLoop> self{this->shared_from_this()};
            acceptor.async_accept([self](const asio::error_code& error,
                                         asio::ip::tcp::socket socket) {
                if (!self->running) {
                    return;
                }
                if (error) {
                    if (error != asio::error::operation_aborted) {
                        self->reportError(error);
                        self->acceptNext();
                    }
                    return;
                }

                try {
                    std::shared_ptr<ConnectionType> connection{
                        self->connectionFactory(std::move(socket))};
                    if (connection) {
                        self->acceptCallback(std::move(connection));
                    }
                } catch (const std::exception& e) {
                    LOG_ERROR("Failed to construct connection: %s", e.what());
                    self->reportError(
                        asio::error::make_error_code(asio::error::fault));
                }

                self->acceptNext();
            });
        }

        void reportError(const asio::error_code& error)
        {
            if (errorCallback) {
                errorCallback(error);
            }
        }

        asio::ip::tcp::acceptor acceptor;
        asio::ip::tcp::endpoint endpoint;
        ConnectionFactory connectionFactory;
        AcceptCallback acceptCallback;
        ErrorCallback errorCallback;
        bool running;
    };

    std::shared_ptr<AcceptLoop> acceptLoop;
};

} // namespace AM
