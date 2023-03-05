#pragma once

// Use the project's Server::Config, if one is provided.
#if defined(AM_OVERRIDE_DEFAULT_CONFIGS)
#include "Override/ServerConfig.h"
#else
#include "SharedConfig.h"
#include "ConstexprTools.h"
#include "SpawnStrategy.h"
#include <SDL_stdinc.h>
#include <string>

namespace AM
{
namespace Server
{
/**
 * Contains configuration constants relevant to the Server.
 */
class Config
{
public:
    //-------------------------------------------------------------------------
    // Simulation
    //-------------------------------------------------------------------------
    /** The strategy that we'll use when spawning entities. */
    static constexpr SpawnStrategy SPAWN_STRATEGY{SpawnStrategy::Fixed};

    // SpawnPoint::Fixed Config
    /** Fixed spawn position. */
    static constexpr float SPAWN_POINT_FIXED_X{32};
    static constexpr float SPAWN_POINT_FIXED_Y{32};

    // SpawnPoint::Random Config
    // Note: We offset these positions to keep entities in bounds while moving.
    /** The min and max X positions that entities will be spawned within. */
    static constexpr float SPAWN_POINT_RANDOM_MIN_X{
        SharedConfig::TILE_WORLD_WIDTH};
    static constexpr float SPAWN_POINT_RANDOM_MAX_X{
        static_cast<unsigned int>(SharedConfig::AOI_RADIUS)
        - SharedConfig::TILE_WORLD_WIDTH};

    /** The min and max Y positions that entities will be spawned within. */
    static constexpr float SPAWN_POINT_RANDOM_MIN_Y{
        SharedConfig::TILE_WORLD_WIDTH};
    static constexpr float SPAWN_POINT_RANDOM_MAX_Y{
        static_cast<unsigned int>(SharedConfig::AOI_RADIUS)
        - SharedConfig::TILE_WORLD_WIDTH};

    // SpawnPoint::Grouped Config
    /** The starting position of the first group. */
    static constexpr float SPAWN_POINT_GROUP_MIN_X{SPAWN_POINT_RANDOM_MIN_X};
    static constexpr float SPAWN_POINT_GROUP_MIN_Y{SPAWN_POINT_RANDOM_MIN_Y};

    /** The number of columns and rows that a group consists of. */
    static constexpr unsigned int SPAWN_POINT_GROUP_COLUMNS{5};
    static constexpr unsigned int SPAWN_POINT_GROUP_ROWS{2};

    /** The amount of padding between rows and columns within a group. */
    static constexpr float SPAWN_POINT_GROUP_PADDING_X{32};
    static constexpr float SPAWN_POINT_GROUP_PADDING_Y{32};

    /** The offset between groups. */
    static constexpr float SPAWN_POINT_GROUP_OFFSET_X{0};
    static constexpr float SPAWN_POINT_GROUP_OFFSET_Y{400};

    /** How often the world's tile map should be saved, in seconds. */
    static constexpr float MAP_SAVE_PERIOD_S{60 * 15};

    //-------------------------------------------------------------------------
    // Network
    //-------------------------------------------------------------------------
    ////////////////////////////////////////////////////////////////////////////
    // A tick diff is the difference between a message's intended tick and our
    // current tick (a positive diff means the client is ahead of the server).
    //
    // Client connection management works as such:
    // 1. If the client is too far ahead or behind, send an adjustment.
    //    (See TICKDIFF_ACCEPTABLE_* and TICKDIFF_TARGET.)
    //
    // 2. If a single tickdiff is lower than TICKDIFF_MAX_BOUND_LOWER or
    //    higher than TICKDIFF_MAX_BOUND_UPPER, disconnect them.
    //
    // 3. If we haven't received data from the client within CLIENT_TIMEOUT_S
    //    seconds, disconnect them.
    ////////////////////////////////////////////////////////////////////////////

    /** The port that the server listens for incoming client connections on. */
    static constexpr unsigned int SERVER_PORT{41499};

    /** The maximum number of clients that we will allow. */
    static constexpr unsigned int MAX_CLIENTS{1010};

    /** How long we should wait before considering the client to be timed out.
        Arbitrarily chosen. If too high, we set ourselves up to take a huge
       spike of data for a very late client. */
    static constexpr double CLIENT_TIMEOUT_S{1.5};

    /** The minimum amount of time worth of tick differences that we'll
        remember. */
    static constexpr double TICKDIFF_HISTORY_S{.5};
    /** The integer number of diffs that we'll remember in the history. */
    static constexpr unsigned int TICKDIFF_HISTORY_LENGTH{
        ConstexprTools::ceilInt(TICKDIFF_HISTORY_S
                                / SharedConfig::SIM_TICK_TIMESTEP_S)};

    /** The range of difference (inclusive) between a received message's tickNum
        and our current tickNum that we won't send an adjustment for. */
    static constexpr Sint64 TICKDIFF_ACCEPTABLE_BOUND_LOWER{1};
    static constexpr Sint64 TICKDIFF_ACCEPTABLE_BOUND_UPPER{3};
    /** The value that we'll adjust clients to if they fall outside the bounds.
     */
    static constexpr Sint64 TICKDIFF_TARGET{2};

    /** The range of difference (inclusive) between a received message's tickNum
        and our current tickNum that will cause us to disconnect a client. */
    static constexpr Sint64 TICKDIFF_MAX_BOUND_LOWER{SDL_MIN_SINT8};
    static constexpr Sint64 TICKDIFF_MAX_BOUND_UPPER{ConstexprTools::ceilInt(
        CLIENT_TIMEOUT_S / SharedConfig::SIM_TICK_TIMESTEP_S)};
    static_assert(TICKDIFF_MAX_BOUND_UPPER <= SDL_MAX_SINT8,
                  "Bound is too high (must fit in SINT8).");

    /** The minimum number of fresh diffs we'll use to calculate an adjustment.
        Aims to prevent thrashing. */
    static constexpr unsigned int MIN_FRESH_DIFFS{3};
};

} // End namespace Server
} // End namespace AM

#endif // defined(AM_OVERRIDE_CONFIG)
