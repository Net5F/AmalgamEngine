#pragma once

#include "AccountClientEndpoint.h"
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

/** Owns and runs the Client's network endpoints. */
class Network
{
private:
    using WorkGuard
        = asio::executor_work_guard<asio::io_context::executor_type>;

    asio::io_context ioContext;
    asio::ssl::context sslContext;

public:
    Network(const MessageProcessorContext& inMessageProcessorContext);

    ~Network();

    /** Starts the asynchronous endpoint thread. */
    void start();

    WorldClientEndpoint worldClientEndpoint;
    AccountClientEndpoint accountClientEndpoint;

private:
    void stop();

    std::optional<WorkGuard> workGuard;
    std::jthread ioThread;
    bool started;
};

} // namespace Client
} // namespace AM
