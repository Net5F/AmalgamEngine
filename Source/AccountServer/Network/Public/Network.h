#pragma once

#include "AccountClientEndpoint.h"
#include "AccountServiceEndpoint.h"
#include "TlsHelpers.h"
#include "asio/ssl/context.hpp"
#include "asio/steady_timer.hpp"
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
     * Loads the pins of each service that's allowed to connect to us.
     */
    static AccountServiceEndpoint::ServicePins loadServicePins();

    /**
     * Configures serviceSSLContext to require a client certificate that
     * matches one of our service pins.
     */
    void configureServicePeerVerification();

    /**
     * Deletes expired sessions and service tickets on the database pool, then
     * schedules the next cleanup.
     */
    void cleanUpExpiredSessions();

    /** Shared network event queue for all endpoints. */
    asio::io_context& ioContext;

    Database& database;

    /** Used by clientEndpoint. Doesn't authenticate peers. */
    asio::ssl::context clientSSLContext;

    /** Used by serviceEndpoint. Requires peers to present a pinned
        certificate (mutual TLS). */
    asio::ssl::context serviceSSLContext;

    asio::thread_pool databasePool;

    /** Fires every SESSION_CLEANUP_INTERVAL_S to trigger
        cleanUpExpiredSessions(). */
    asio::steady_timer sessionCleanupTimer;

    /** The pins of each service that's allowed to connect to us, indexed by
        service. */
    AccountServiceEndpoint::ServicePins servicePins;

    AccountClientEndpoint clientEndpoint;

    AccountServiceEndpoint serviceEndpoint;
};

} // End namespace AccountServer
} // End namespace AM
