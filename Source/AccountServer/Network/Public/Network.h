#pragma once

#include "AccountClientEndpoint.h"
#include "AccountServiceEndpoint.h"

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

    /**
     * Starts our network endpoint management.
     */
    void start();

private:
    /** Shared network event queue for all endpoints. */
    asio::io_context& ioContext;

    AccountClientEndpoint accountClientEndpoint;

    AccountServiceEndpoint accountServiceEndpoint;
};

} // namespace AccountServer
} // namespace AM
