#pragma once

#include "AccountClientEndpoint.h"
#include "ChatClientEndpoint.h"
#include "WorldClientEndpoint.h"
#include "asio/executor_work_guard.hpp"
#include "asio/io_context.hpp"
#include "asio/ssl/context.hpp"
#include <optional>
#include <thread>

namespace AM
{
namespace Client
{
struct MessageProcessorContext;

/**
 * Owns and runs the Client's network endpoints.
 */
class Network
{
private:
    asio::io_context ioContext;
    /** Each server has its own pinned certificate, so each needs its own
        context. */
    asio::ssl::context accountSSLContext;
    asio::ssl::context chatSSLContext;

public:
    Network(const MessageProcessorContext& inMessageProcessorContext);

    ~Network();

    /** Starts the asynchronous endpoint thread. */
    void start();

    WorldClientEndpoint worldEndpoint;
    AccountClientEndpoint accountEndpoint;
    ChatClientEndpoint chatEndpoint;

private:
    using WorkGuard
        = asio::executor_work_guard<asio::io_context::executor_type>;

    void stop();

    /**
     * Attempts to load the given certificate pin and set it for use in the
     * given context.
     */
    void loadCertificatePin(asio::ssl::context& sslContext,
                            const std::string& fileName,
                            const std::string& serverName);

    std::optional<WorkGuard> workGuard;
    std::jthread ioThread;
    bool started;
};

} // namespace Client
} // namespace AM
