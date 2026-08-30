#include "Network.h"
#include "Database.h"
#include "asio/io_context.hpp"

namespace AM
{
namespace AccountServer
{

Network::Network(asio::io_context& inIoContext, Database& inDatabase)
: ioContext{inIoContext}
, sslContext{asio::ssl::context::tls_server}
, databasePool{1}
, accountClientEndpoint{inIoContext, sslContext, databasePool, inDatabase}
, accountServiceEndpoint{}
{
}

Network::~Network()
{
    databasePool.join();
}

void Network::start()
{
    accountClientEndpoint.start();

    // accountClientEndpoint and accountServiceEndpoint both use this context
    // for their events. By running it, we run them all.
    ioContext.run();
}

} // namespace AccountServer
} // namespace AM
