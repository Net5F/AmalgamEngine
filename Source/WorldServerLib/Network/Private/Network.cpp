#include "Network.h"
#include "MessageProcessorContext.h"
#include "TlsHelpers.h"
#include "Paths.h"
#include "Log.h"
#include "asio/system_error.hpp"
#include "tracy/Tracy.hpp"
#include <openssl/ssl.h>
#include <string>

namespace AM
{
namespace WorldServer
{

Network::Network(const MessageProcessorContext& inMessageProcessorContext)
: ioContext{}
, accountSSLContext{asio::ssl::context::tls_client}
, accountEndpoint{ioContext, accountSSLContext}
, clientEndpoint{inMessageProcessorContext, accountEndpoint}
, workGuard{}
, ioThread{}
, started{false}
{
    loadServiceCredentials();
    configureAccountServerVerification();
}

Network::~Network()
{
    stop();
}

void Network::start()
{
    if (started) {
        LOG_INFO("Attempted to start WorldServer networking more than once.");
        return;
    }

    ioContext.restart();
    workGuard.emplace(asio::make_work_guard(ioContext));
    ioThread = std::jthread([this]() {
        tracy::SetThreadName("ServerAsyncNetwork");
        ioContext.run();
    });
    started = true;

    accountEndpoint.start();
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

void Network::loadServiceCredentials()
{
    const std::string certificatePath{Paths::BASE_PATH + "world-server.crt"};
    const std::string privateKeyPath{Paths::BASE_PATH + "world-server.key"};

    try {
        accountSSLContext.use_certificate_chain_file(certificatePath);
    } catch (const asio::system_error& e) {
        LOG_FATAL("Failed to load WorldServer certificate '%s': %s",
                  certificatePath.c_str(), e.what());
    }

    try {
        accountSSLContext.use_private_key_file(privateKeyPath,
                                               asio::ssl::context::pem);
    } catch (const asio::system_error& e) {
        LOG_FATAL("Failed to load WorldServer private key '%s': %s",
                  privateKeyPath.c_str(), e.what());
    }

    if (SSL_CTX_check_private_key(accountSSLContext.native_handle()) != 1) {
        LOG_FATAL("WorldServer certificate and private key do not match: "
                  "'%s', '%s'",
                  certificatePath.c_str(), privateKeyPath.c_str());
    }
}

void Network::configureAccountServerVerification()
{
    const std::string pinPath{Paths::BASE_PATH + "account-server.pin"};
    auto certificatePinOpt{TlsHelpers::loadCertificatePin(pinPath)};
    if (!certificatePinOpt) {
        LOG_FATAL("Failed to load AccountServer certificate pin.");
    }

    // Require the AccountServer to present a certificate that matches the
    // pin. Unauthenticated peers will fail the handshake.
    try {
        accountSSLContext.set_verify_mode(asio::ssl::verify_peer);
        accountSSLContext.set_verify_callback(
            [certificatePin = *certificatePinOpt](
                bool, asio::ssl::verify_context& verifyContext) noexcept {
                return TlsHelpers::verifyPinnedCertificate(
                    certificatePin, verifyContext,
                    TlsHelpers::PeerRole::Server);
            });
    } catch (const asio::system_error& e) {
        LOG_FATAL("Failed to configure AccountServer certificate pin '%s': %s",
                  pinPath.c_str(), e.what());
    }
}

} // namespace WorldServer
} // namespace AM
