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

    /** Our best guess at a good starting place for the simulation's tick
       offset. Doesn't matter much since the server will quickly adjust us. */
    static constexpr Sint8 INITIAL_TICK_OFFSET = 5;

    /** How long we should wait before considering the server to be timed out.
     */
    static constexpr double SERVER_TIMEOUT_S
        = SharedConfig::NETWORK_TICK_TIMESTEP_S * 2;
};

} // End namespace Client
} // End namespace AM
