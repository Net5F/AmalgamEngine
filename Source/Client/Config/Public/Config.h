#pragma once

#include "SharedConfig.h"
#include <SDL2/SDL_stdinc.h>
#include <string>

namespace AM
{
namespace Client
{
/**
 * This class contains module-specific configuration data.
 *
 * All data is currently static, but eventually this class will be in charge
 * of loading some of the data dynamically from a config file.
 */
class Config
{
public:
    //-------------------------------------------------------------------------
    // Network
    //-------------------------------------------------------------------------
    /** If true, the connection to the server will be mocked and we'll run
       without it. */
    static constexpr bool RUN_OFFLINE = false;

    static const std::string SERVER_IP;
    static constexpr unsigned int SERVER_PORT = 41499;

    /** How far our client's simulation will be ahead of the server's
        simulation.
        We start with this offset, then keep ourselves ahead by using the
        adjustments that the server sends us.
        Doesn't matter much since the server will quickly adjust us. */
    static constexpr Sint8 INITIAL_TICK_OFFSET = 5;

    /** How far into the past to begin replicating NPCs at.
        We negate INITIAL_TICK_OFFSET since we want to be in the past instead
        of in the future. Additionally, we double it since the messages we
        receive will appear to be doubly far into the past (since we're in the
        future.) */
    static constexpr Sint8 INITIAL_REPLICATION_OFFSET = -2 * INITIAL_TICK_OFFSET;

    /** How long we should wait before considering the server to be timed out.
     */
    static constexpr double SERVER_TIMEOUT_S
        = SharedConfig::NETWORK_TICK_TIMESTEP_S * 2;
};

} // End namespace Client
} // End namespace AM
