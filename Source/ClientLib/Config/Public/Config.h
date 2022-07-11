#pragma once

// Use the project's Client::Config, if one is provided.
#if defined(AM_OVERRIDE_DEFAULT_CONFIGS)
#include "Override/ClientConfig.h"
#else
#include "SharedConfig.h"
#include <SDL_stdinc.h>
#include <string>

namespace AM
{
namespace Client
{
/**
 * Contains configuration constants relevant to the Client.
 */
class Config
{
public:
    //-------------------------------------------------------------------------
    // Network
    //-------------------------------------------------------------------------
    /** If true, the connection to the server will be mocked and we'll run
        without it. */
    static constexpr bool RUN_OFFLINE{false};

    // static constexpr char SERVER_IP[] = "127.0.0.1";
    static constexpr char SERVER_IP[] = "104.237.139.17";
    static constexpr unsigned int SERVER_PORT{41499};

    /** How far our client's simulation will be ahead of the server's
        simulation.
        We start with this offset, then keep ourselves ahead by using the
        adjustments that the server sends us.
        Doesn't matter much since the server will quickly adjust us. */
    static constexpr Sint8 INITIAL_TICK_OFFSET{5};

    /** How far into the past to begin replicating non-predicted state at.
        We negate INITIAL_TICK_OFFSET since we want to be in the past instead
        of in the future. Additionally, we double it since the messages we
        receive will appear to be doubly far into the past (since we're in the
        future.) */
    static constexpr Sint8 INITIAL_REPLICATION_OFFSET{-2 * INITIAL_TICK_OFFSET};

    /** How long we should wait before considering the server to be timed out.
     */
    static constexpr double SERVER_TIMEOUT_S{
        SharedConfig::NETWORK_TICK_TIMESTEP_S * 2};

    //-------------------------------------------------------------------------
    // Renderer, User Interface
    //-------------------------------------------------------------------------
    /** The actual size of the window we're rendering to. */
    static constexpr unsigned int ACTUAL_SCREEN_WIDTH{1600};
    static constexpr unsigned int ACTUAL_SCREEN_HEIGHT{900};

    /**
     * The logical screen size that we're targeting.
     *
     * The developer can write layout logic in reference to this logical size,
     * and it will be scaled to match the actual size above.
     */
    static constexpr unsigned int LOGICAL_SCREEN_WIDTH{1920};
    static constexpr unsigned int LOGICAL_SCREEN_HEIGHT{1080};

    /**
     * Sets full screen preference.
     * 0 = windowed.
     * 1 = Real fullscreen.
     * 2 = Fullscreen windowed.
     */
    static constexpr unsigned int FULLSCREEN_MODE{0};

    /**
     * Sets the quality of scaling algorithm used.
     * "nearest" = Nearest pixel sampling.
     * "linear" = Linear filtering (supported by OpenGL and Direct3D).
     * "best" = Ansiotropic filtering (supported by Direct3D).
     */
    static constexpr char SCALING_QUALITY[] = "linear";

    /** The rate at which the user interfaces fires off tick() events. */
    static constexpr unsigned int UI_TICKS_PER_SECOND{30};
    static constexpr double UI_TICK_TIMESTEP_S{
        1.0 / static_cast<double>(UI_TICKS_PER_SECOND)};
};

} // End namespace Client
} // End namespace AM

#endif // defined(AM_OVERRIDE_CONFIG)
