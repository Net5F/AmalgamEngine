#include "Network.h"
#include "Database.h"
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
, sslContext{asio::ssl::context::tls_server}
, databasePool{1}
, clientEndpoint{inIoContext, sslContext, databasePool, inDatabase}
, serviceEndpoint{}
{
    const std::string certificatePath{Paths::BASE_PATH + "account-server.crt"};
    const std::string privateKeyPath{Paths::BASE_PATH + "account-server.key"};

    try {
        sslContext.use_certificate_chain_file(certificatePath);
    } catch (const asio::system_error& e) {
        LOG_FATAL("Failed to load AccountServer certificate '%s': %s",
                  certificatePath.c_str(), e.what());
    }

    try {
        sslContext.use_private_key_file(privateKeyPath,
                                        asio::ssl::context::pem);
    } catch (const asio::system_error& e) {
        LOG_FATAL("Failed to load AccountServer private key '%s': %s",
                  privateKeyPath.c_str(), e.what());
    }

    if (SSL_CTX_check_private_key(sslContext.native_handle()) != 1) {
        LOG_FATAL("AccountServer certificate and private key do not match: "
                  "'%s', '%s'",
                  certificatePath.c_str(), privateKeyPath.c_str());
    }
}

Network::~Network()
{
    databasePool.join();
}

void Network::start()
{
    clientEndpoint.start();

    // clientEndpoint and serviceEndpoint both use this context
    // for their events. By running it, we run them all.
    LOG_INFO("Starting network IO context.");
    ioContext.run();
}

} // namespace AccountServer
} // namespace AM
