#pragma once

#include "WorldClientEndpoint.h"
#include "ChatServiceEndpoint.h"
#include "Shared/AccountServiceEndpoint.h"
#include "asio/executor_work_guard.hpp"
#include "asio/io_context.hpp"
#include "asio/ssl/context.hpp"
#include <optional>
#include <string>
#include <thread>

namespace AM
{
namespace WorldServer
{
struct MessageProcessorContext;

/**
 * Owns WorldServer's network endpoints.
 *
 * Also owns and runs the asynchronous IO thread used by the asio-based
 * endpoints.
 */
class Network
{
private:
    asio::io_context ioContext;

    /** Used by accountEndpoint. Presents our certificate and requires
        AccountServer to present its pinned certificate (mutual TLS). */
    asio::ssl::context accountSSLContext;

    /** Used by chatEndpoint. Presents our certificate and requires
        ChatServer to present its pinned certificate (mutual TLS). */
    asio::ssl::context chatSSLContext;

public:
    Network(const MessageProcessorContext& inMessageProcessorContext);

    ~Network();

    /** Starts the asynchronous endpoint thread. */
    void start();

    // Note: accountEndpoint must be constructed before clientEndpoint, since
    //       clientEndpoint starts threads that use it.
    AccountServiceEndpoint accountEndpoint;
    WorldClientEndpoint clientEndpoint;
    ChatServiceEndpoint chatEndpoint;

private:
    using WorkGuard
        = asio::executor_work_guard<asio::io_context::executor_type>;

    void stop();

    /**
     * Loads our certificate and private key into the given context.
     */
    void loadServiceCredentials(asio::ssl::context& context);

    /**
     * Configures the given context to require the server's certificate to
     * match the pin in the given file.
     *
     * @param serverName The server's name, for logging.
     */
    void configureServerVerification(asio::ssl::context& context,
                                     const std::string& pinFileName,
                                     const char* serverName);

    std::optional<WorkGuard> workGuard;
    std::jthread ioThread;
    bool started;
};

} // namespace WorldServer
} // namespace AM
