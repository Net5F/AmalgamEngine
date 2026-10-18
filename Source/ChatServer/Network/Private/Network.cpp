#include "Network.h"
#include "UserConfig.h"
#include "TlsHelpers.h"
#include "Paths.h"
#include "Log.h"
#include "asio/io_context.hpp"
#include "asio/system_error.hpp"
#include <openssl/ssl.h>
#include <string>

namespace AM
{
namespace ChatServer
{

Network::Network(asio::io_context& inIoContext)
: ioContext{inIoContext}
, clientSSLContext{asio::ssl::context::tls_server}
, serviceSSLContext{asio::ssl::context::tls_server}
, accountSSLContext{asio::ssl::context::tls_client}
, accountEndpoint{inIoContext, accountSSLContext,
                  UserConfig::get().getAccountServerAddress().IP,
                  UserConfig::get().getAccountServerAddress().port}
, playerLocator{}
, clientEndpoint{inIoContext, clientSSLContext, accountEndpoint, playerLocator}
, serviceEndpoint{inIoContext, serviceSSLContext, playerLocator}
{
    loadCredentials(clientSSLContext);
    loadCredentials(serviceSSLContext);
    loadCredentials(accountSSLContext);
    configureServicePeerVerification();
    configureAccountServerVerification();
}

void Network::start()
{
    accountEndpoint.start();
    clientEndpoint.start();
    serviceEndpoint.start();

    // All of our endpoints use this context for their events. By running it,
    // we run them all.
    LOG_INFO("Starting network IO context.");
    ioContext.run();
}

void Network::loadCredentials(asio::ssl::context& context)
{
    const std::string certificatePath{Paths::BASE_PATH + "chat-server.crt"};
    const std::string privateKeyPath{Paths::BASE_PATH + "chat-server.key"};

    try {
        context.use_certificate_chain_file(certificatePath);
    } catch (const asio::system_error& e) {
        LOG_FATAL("Failed to load ChatServer certificate '%s': %s",
                  certificatePath.c_str(), e.what());
    }

    try {
        context.use_private_key_file(privateKeyPath, asio::ssl::context::pem);
    } catch (const asio::system_error& e) {
        LOG_FATAL("Failed to load ChatServer private key '%s': %s",
                  privateKeyPath.c_str(), e.what());
    }

    if (SSL_CTX_check_private_key(context.native_handle()) != 1) {
        LOG_FATAL("ChatServer certificate and private key do not match: "
                  "'%s', '%s'",
                  certificatePath.c_str(), privateKeyPath.c_str());
    }
}

void Network::configureServicePeerVerification()
{
    const std::string pinPath{Paths::BASE_PATH + "world-server.pin"};
    auto certificatePinOpt{TlsHelpers::loadCertificatePin(pinPath)};
    if (!certificatePinOpt) {
        LOG_FATAL("Failed to load WorldServer certificate pin.");
    }

    // Require connecting services to present a certificate that matches the
    // pin. Unauthenticated peers will fail the handshake.
    try {
        serviceSSLContext.set_verify_mode(
            asio::ssl::verify_peer | asio::ssl::verify_fail_if_no_peer_cert);
        serviceSSLContext.set_verify_callback(
            [certificatePin = *certificatePinOpt](
                bool, asio::ssl::verify_context& verifyContext) noexcept {
                return TlsHelpers::verifyPinnedCertificate(
                    certificatePin, verifyContext,
                    TlsHelpers::PeerRole::Client);
            });
    } catch (const asio::system_error& e) {
        LOG_FATAL("Failed to configure WorldServer certificate pin '%s': %s",
                  pinPath.c_str(), e.what());
    }
}

void Network::configureAccountServerVerification()
{
    const std::string pinPath{Paths::BASE_PATH + "account-server.pin"};
    auto certificatePinOpt{TlsHelpers::loadCertificatePin(pinPath)};
    if (!certificatePinOpt) {
        LOG_FATAL("Failed to load AccountServer certificate pin.");
    }

    // Require AccountServer to present a certificate that matches the
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

} // End namespace ChatServer
} // End namespace AM
