#include "Network.h"
#include "MessageProcessorContext.h"
#include "StringTools.h"
#include "TlsHelpers.h"
#include "Paths.h"
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
    // Load and set each server's pin.
    loadCertificatePin("account-server.pin", "AccountServer");
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

void Network::loadCertificatePin(const std::string& fileName,
                                 const std::string& serverName)
{
    const std::string pinPath{Paths::BASE_PATH + fileName};
    auto certificatePinOpt{TlsHelpers::loadCertificatePin(pinPath)};
    if (!certificatePinOpt) {
        LOG_FATAL("Failed to load %s certificate pin.", serverName.c_str());
    }

    try {
        sslContext.set_verify_mode(asio::ssl::verify_peer);
        sslContext.set_verify_callback(
            [certificatePin = *certificatePinOpt](
                bool, asio::ssl::verify_context& verifyContext) noexcept {
                return TlsHelpers::verifyPinnedCertificate(certificatePin,
                                                           verifyContext);
            });
    } catch (const asio::system_error& e) {
        LOG_FATAL("Failed to configure %s certificate pin '%s': "
                  "%s",
                  serverName.c_str(), pinPath.c_str(), e.what());
    }
}

} // namespace Client
} // namespace AM
