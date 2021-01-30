#pragma once

#include <SDL_stdinc.h>
#include <array>

/**
 * This file contains shared game definitions that should be
 * consistent between the server and client.
 */
namespace AM
{
//--------------------------------------------------------------------------
// Config
//--------------------------------------------------------------------------
static constexpr unsigned int SIM_TICKS_PER_SECOND = 30;
static constexpr double SIM_TICK_TIMESTEP_S
    = 1.0 / static_cast<double>(SIM_TICKS_PER_SECOND);

// TODO: Move these out.
static constexpr unsigned int SCREEN_WIDTH = 1280;
static constexpr unsigned int SCREEN_HEIGHT = 720;

/** The max number of entities that we are allowing. */
static constexpr unsigned int MAX_ENTITIES = 1010;

/** The distance in world coordinates that we add to a player's AoI to let
    peers load before they're in view. */
static constexpr unsigned int AOI_BUFFER_DISTANCE = 700;

/** The width of the world tiles in world coordinates. */
static constexpr unsigned int TILE_WORLD_WIDTH = 32;
/** The height of the world tiles in world coordinates. */
static constexpr unsigned int TILE_WORLD_HEIGHT = 32;

/** The width of the screen tiles in screen coordinates. */
static constexpr unsigned int TILE_SCREEN_WIDTH = 256;
/** The height of the screen tiles in screen coordinates. */
static constexpr unsigned int TILE_SCREEN_HEIGHT = 128;

/** The width of the world's terrain map in tiles. */
static constexpr unsigned int WORLD_WIDTH = 10;
/** The height of the world's terrain map in tiles. */
static constexpr unsigned int WORLD_HEIGHT = 10;

} /* End namespace AM */
