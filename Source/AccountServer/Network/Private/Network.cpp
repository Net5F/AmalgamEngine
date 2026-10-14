#include "Network.h"
#include "Database.h"
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
namespace AccountServer
{

Network::Network(asio::io_context& inIoContext, Database& inDatabase)
: ioContext{inIoContext}
, clientSSLContext{asio::ssl::context::tls_server}
, serviceSSLContext{asio::ssl::context::tls_server}
, databasePool{UserConfig::get().getDatabaseWorkerCount()}
, servicePins{loadServicePins()}
, clientEndpoint{inIoContext, clientSSLContext, databasePool, inDatabase}
, serviceEndpoint{inIoContext, serviceSSLContext, databasePool, inDatabase,
                  servicePins}
{
    loadServerCredentials(clientSSLContext);
    loadServerCredentials(serviceSSLContext);
    configureServicePeerVerification();
}

Network::~Network()
{
    databasePool.join();
}

void Network::start()
{
    clientEndpoint.start();
    serviceEndpoint.start();

    // clientEndpoint and serviceEndpoint both use this context
    // for their events. By running it, we run them all.
    LOG_INFO("Starting network IO context.");
    ioContext.run();
}

void Network::loadServerCredentials(asio::ssl::context& context)
{
    const std::string certificatePath{Paths::BASE_PATH + "account-server.crt"};
    const std::string privateKeyPath{Paths::BASE_PATH + "account-server.key"};

    try {
        context.use_certificate_chain_file(certificatePath);
    } catch (const asio::system_error& e) {
        LOG_FATAL("Failed to load AccountServer certificate '%s': %s",
                  certificatePath.c_str(), e.what());
    }

    try {
        context.use_private_key_file(privateKeyPath, asio::ssl::context::pem);
    } catch (const asio::system_error& e) {
        LOG_FATAL("Failed to load AccountServer private key '%s': %s",
                  privateKeyPath.c_str(), e.what());
    }

    if (SSL_CTX_check_private_key(context.native_handle()) != 1) {
        LOG_FATAL("AccountServer certificate and private key do not match: "
                  "'%s', '%s'",
                  certificatePath.c_str(), privateKeyPath.c_str());
    }
}

AccountServiceEndpoint::ServicePins Network::loadServicePins()
{
    AccountServiceEndpoint::ServicePins pins{};
    for (std::size_t i{0}; i < pins.size(); ++i) {
        auto service{static_cast<ServiceTicketAudience>(i)};
        const char* pinFileName{};
        switch (service) {
            case ServiceTicketAudience::WorldServer: {
                pinFileName = "world-server.pin";
                break;
            }
            case ServiceTicketAudience::ChatServer: {
                pinFileName = "chat-server.pin";
                break;
            }
            default: {
                LOG_FATAL("Missing pin file for service: %u",
                          static_cast<unsigned int>(service));
            }
        }

        const std::string pinPath{Paths::BASE_PATH + pinFileName};
        auto certificatePinOpt{TlsHelpers::loadCertificatePin(pinPath)};
        if (!certificatePinOpt) {
            LOG_FATAL("Failed to load service certificate pin '%s'.",
                      pinPath.c_str());
        }

        pins[i] = *certificatePinOpt;
    }

    return pins;
}

void Network::configureServicePeerVerification()
{
    // Require connecting services to present a certificate that matches one
    // of our pins. Unauthenticated peers will fail the handshake.
    // Note: Once the handshake completes, serviceEndpoint checks the peer's
    //       pin again to identify which service it is.
    try {
        serviceSSLContext.set_verify_mode(
            asio::ssl::verify_peer | asio::ssl::verify_fail_if_no_peer_cert);
        serviceSSLContext.set_verify_callback(
            [pins = servicePins](
                bool, asio::ssl::verify_context& verifyContext) noexcept {
                for (const TlsHelpers::CertificatePin& pin : pins) {
                    if (TlsHelpers::verifyPinnedCertificate(
                            pin, verifyContext, TlsHelpers::PeerRole::Client)) {
                        return true;
                    }
                }
                return false;
            });
    } catch (const asio::system_error& e) {
        LOG_FATAL("Failed to configure service certificate pins: %s", e.what());
    }
}

} // End namespace AccountServer
} // End namespace AM
