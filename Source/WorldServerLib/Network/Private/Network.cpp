#include "Network.h"
#include "MessageProcessorContext.h"

namespace AM
{
namespace WorldServer
{

Network::Network(const MessageProcessorContext& inMessageProcessorContext)
: clientEndpoint{inMessageProcessorContext}
{
}

} // namespace WorldServer
} // namespace AM
