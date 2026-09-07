#include "Network.h"
#include "MessageProcessorContext.h"
#include "Log.h"

namespace AM
{
namespace Client
{

Network::Network(const MessageProcessorContext& inMessageProcessorContext)
: ioContext{}
, sslContext{asio::ssl::context::tls_client}
, worldClientEndpoint{inMessageProcessorContext}
, accountClientEndpoint{ioContext, sslContext, inMessageProcessorContext}
, workGuard{}
, ioThread{}
, started{false}
{
}

Network::~Network()
{
    stop();
}

void Network::start()
{
    if (started) {
        LOG_INFO("Attempted to start Client networking more than once.");
        return;
    }

    ioContext.restart();
    workGuard.emplace(asio::make_work_guard(ioContext));
    ioThread = std::jthread([this]() { ioContext.run(); });
    started = true;
}

void Network::stop()
{
    if (!started) {
        return;
    }

    workGuard.reset();
    ioContext.stop();
    if (ioThread.joinable()) {
        ioThread.join();
    }
    started = false;
}

} // namespace Client
} // namespace AM
