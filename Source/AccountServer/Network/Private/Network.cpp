#include "Network.h"
#include "Database.h"
#include "asio/io_context.hpp"

namespace AM
{
namespace AccountServer
{

Network::Network(asio::io_context& inIoContext, Database& inDatabase)
: ioContext{inIoContext}
, accountClientEndpoint{}
, accountServiceEndpoint{}
{
}

void Network::start()
{
    // TODO: Start acceptor

    // accountClientEndpoint and accountServiceEndpoint both use this context
    // for their events. By running it, we run them all.
    ioContext.run();
}

} // namespace AccountServer
} // namespace AM
