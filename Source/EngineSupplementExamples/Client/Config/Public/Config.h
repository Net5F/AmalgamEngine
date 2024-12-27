#pragma once

#include "SharedConfig.h"
#include <SDL_stdinc.h>
#include <SDL_render.h>
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
        SharedConfig::SERVER_NETWORK_TICK_TIMESTEP_S * 2};

    //-------------------------------------------------------------------------
    // Simulation
    //-------------------------------------------------------------------------
    /** The x and y axis width, in tiles, of a cell in our world object spatial
        partitioning structure (WorldObjectLocator).
        Note: The tile map's size must be evenly divisible by this number. */
    static constexpr std::size_t WORLD_OBJECT_LOCATOR_CELL_WIDTH{4};

    /** The z axis height, in tiles, of a cell in our world object spatial
        partitioning structure (WorldObjectLocator).
        Note: The tile map's size must be evenly divisible by this number. */
    static constexpr std::size_t WORLD_OBJECT_LOCATOR_CELL_HEIGHT{2};

    //-------------------------------------------------------------------------
    // Renderer, User Interface
    //-------------------------------------------------------------------------
    /**
     * The title that will show on the application window.
     */
    static constexpr const char* WINDOW_TITLE{"Amalgam"};

    /**
     * The logical screen size that we're targeting.
     *
     * The developer can write layout logic in reference to this logical size,
     * and it will be scaled to match the actual size above.
     */
    static constexpr unsigned int LOGICAL_SCREEN_WIDTH{1920};
    static constexpr unsigned int LOGICAL_SCREEN_HEIGHT{1080};

    /**
     * Sets the quality of scaling/filtering algorithm used for sprite 
     * textures (i.e any textures rendered by Renderer).
     * 
     * UI textures ignore this setting.
     */
    static constexpr SDL_ScaleMode SPRITE_SCALING_QUALITY{SDL_ScaleModeLinear};

    /** The rate at which the user interfaces fires off tick() events. */
    static constexpr unsigned int UI_TICKS_PER_SECOND{30};
    static constexpr double UI_TICK_TIMESTEP_S{
        1.0 / static_cast<double>(UI_TICKS_PER_SECOND)};

    /** We sleep for 1ms when possible to reduce our CPU usage. We can't trust
        the scheduler to come back to us after exactly 1ms though, so we busy
        wait if something needs to be called soon.
        Higher value == more CPU usage.
        If you pick too small of a number, two issues can occur:
          1. If < ~.003, the scheduler may not give us time when we need it.
          2. If < ~.010, our low CPU usage might cause Windows to put us into
             energy saving mode, causing visual jitter. */
    static constexpr double SLEEP_MINIMUM_TIME_S{.010};

    /** If true, zooming with the mouse scroll wheel will be enabled. */
    static constexpr bool ENABLE_MOUSE_ZOOM{false};

    /** The levels of zoom that the player can select. */
    static constexpr std::array<float, 8> ZOOM_LEVELS{0.25, 0.5, 0.75, 1,
                                                      1.25, 1.5, 1.75, 2};

    /** The index in the ZOOM_LEVELS vector that the camera should initially 
        be set to. */
    static constexpr std::size_t DEFAULT_ZOOM_LEVEL_INDEX{3};
};

} // End namespace Client
} // End namespace AM
