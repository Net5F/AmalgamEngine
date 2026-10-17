#pragma once

#include "Shared/AccountServiceEndpoint.h"
#include "ChatClientEndpoint.h"
#include "ChatServiceEndpoint.h"
#include "PlayerLocator.h"
#include "asio/ssl/context.hpp"

namespace asio
{
class io_context;
}

namespace AM
{
namespace ChatServer
{

/**
 * Manages the network connection endpoints.
 */
class Network
{
public:
    Network(asio::io_context& inIoContext);

    /**
     * Starts our network endpoint management.
     */
    void start();

private:
    /**
     * Loads our certificate and private key into the given context.
     */
    void loadCredentials(asio::ssl::context& context);

    /**
     * Configures serviceSSLContext to require a client certificate that
     * matches the world server's pin.
     */
    void configureServicePeerVerification();

    /**
     * Configures accountSSLContext to require the AccountServer's
     * certificate to match its pin.
     */
    void configureAccountServerVerification();

    /** Shared network event queue for all endpoints. */
    asio::io_context& ioContext;

    /** Used by clientEndpoint. Doesn't authenticate peers. */
    asio::ssl::context clientSSLContext;

    /** Used by serviceEndpoint. Requires peers to present a pinned
        certificate (mutual TLS). */
    asio::ssl::context serviceSSLContext;

    /** Used by accountEndpoint. Presents our certificate and requires the
        AccountServer to present its pinned certificate (mutual TLS). */
    asio::ssl::context accountSSLContext;

    // Note: accountEndpoint and playerLocator must be constructed before
    //       clientEndpoint, since clientEndpoint uses them to validate tickets
    //       and route local chat.
    AccountServiceEndpoint accountEndpoint;

    /** Where players are in the world. Updated by serviceEndpoint. */
    PlayerLocator playerLocator;

    ChatClientEndpoint clientEndpoint;

    ChatServiceEndpoint serviceEndpoint;
};

} // End namespace ChatServer
} // End namespace AM
