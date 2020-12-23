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
static constexpr unsigned int GAME_TICKS_PER_SECOND = 30;
static constexpr double GAME_TICK_TIMESTEP_S
    = 1.0 / static_cast<double>(GAME_TICKS_PER_SECOND);

// TODO: Move these out.
static constexpr unsigned int SCREEN_WIDTH = 1280;
static constexpr unsigned int SCREEN_HEIGHT = 720;

/** The max number of entities that we are allowing. */
static constexpr unsigned int MAX_ENTITIES = 200;

} /* End namespace AM */
