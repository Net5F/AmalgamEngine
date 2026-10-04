#pragma once

#include "WorldClientEndpoint.h"
#include "AccountServiceEndpoint.h"
#include "asio/executor_work_guard.hpp"
#include "asio/io_context.hpp"
#include "asio/ssl/context.hpp"
#include <optional>
#include <thread>

namespace AM
{
namespace WorldServer
{
struct MessageProcessorContext;

/**
 * Owns the World Server's network endpoints.
 *
 * Also owns and runs the asynchronous IO thread used by the asio-based
 * endpoints.
 */
class Network
{
private:
    asio::io_context ioContext;

    /** Used by accountEndpoint. Presents our certificate and requires the
        AccountServer to present its pinned certificate (mutual TLS). */
    asio::ssl::context accountSSLContext;

public:
    Network(const MessageProcessorContext& inMessageProcessorContext);

    ~Network();

    /** Starts the asynchronous endpoint thread. */
    void start();

    WorldClientEndpoint clientEndpoint;
    AccountServiceEndpoint accountEndpoint;

private:
    using WorkGuard
        = asio::executor_work_guard<asio::io_context::executor_type>;

    void stop();

    /**
     * Loads our certificate and private key into accountSSLContext.
     */
    void loadServiceCredentials();

    /**
     * Configures accountSSLContext to require the AccountServer's
     * certificate to match its pin.
     */
    void configureAccountServerVerification();

    std::optional<WorkGuard> workGuard;
    std::jthread ioThread;
    bool started;
};

} // namespace WorldServer
} // namespace AM
