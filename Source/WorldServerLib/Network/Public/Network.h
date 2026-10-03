#pragma once

#include "WorldClientEndpoint.h"

namespace AM
{
namespace WorldServer
{
struct MessageProcessorContext;

/**
 * Owns the World Server's network endpoints.
 */
class Network
{
public:
    Network(const MessageProcessorContext& inMessageProcessorContext);

    WorldClientEndpoint clientEndpoint;
};

} // namespace WorldServer
} // namespace AM
