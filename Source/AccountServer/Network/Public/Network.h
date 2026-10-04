#pragma once

#include "AccountClientEndpoint.h"
#include "AccountServiceEndpoint.h"
#include "asio/ssl/context.hpp"
#include "asio/thread_pool.hpp"

namespace asio
{
class io_context;
}

namespace AM
{
namespace AccountServer
{

class Database;

/**
 * Manages the network connection endpoints. 
 */
class Network
{
public:
    Network(asio::io_context& inIoContext, Database& inDatabase);

    ~Network();

    /**
     * Starts our network endpoint management.
     */
    void start();

private:
    /**
     * Loads our certificate and private key into the given context.
     */
    void loadServerCredentials(asio::ssl::context& context);

    /**
     * Configures serviceSSLContext to require a client certificate that
     * matches the world server's pin.
     */
    void configureServicePeerVerification();

    /** Shared network event queue for all endpoints. */
    asio::io_context& ioContext;

    /** Used by clientEndpoint. Doesn't authenticate peers. */
    asio::ssl::context clientSSLContext;

    /** Used by serviceEndpoint. Requires peers to present a pinned
        certificate (mutual TLS). */
    asio::ssl::context serviceSSLContext;

    asio::thread_pool databasePool;

    AccountClientEndpoint clientEndpoint;

    AccountServiceEndpoint serviceEndpoint;
};

} // namespace AccountServer
} // namespace AM
