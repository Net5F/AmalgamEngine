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
    /** Shared network event queue for all endpoints. */
    asio::io_context& ioContext;

    asio::ssl::context sslContext;

    asio::thread_pool databasePool;

    AccountClientEndpoint accountClientEndpoint;

    AccountServiceEndpoint accountServiceEndpoint;
};

} // namespace AccountServer
} // namespace AM
