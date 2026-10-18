#pragma once

#include "SpawnStrategy.h"
#include "SharedConfig.h"
#include "ConstexprTools.h"
#include <SDL3/SDL_stdinc.h>
#include <string>

namespace AM
{
namespace WorldServer
{
/**
 * Contains configuration constants relevant to WorldServer.
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
    // Note: Spawns will start at the NW corner of the map, at Z==0.
    /** The number of columns and rows that a group consists of. */
    static constexpr unsigned int SPAWN_POINT_GROUP_COLUMNS{5};
    static constexpr unsigned int SPAWN_POINT_GROUP_ROWS{2};

    /** The amount of padding between rows and columns within a group. */
    static constexpr float SPAWN_POINT_GROUP_PADDING_X{32};
    static constexpr float SPAWN_POINT_GROUP_PADDING_Y{32};

    /** The offset between groups. */
    static constexpr float SPAWN_POINT_GROUP_OFFSET_X{500};
    static constexpr float SPAWN_POINT_GROUP_OFFSET_Y{500};

    /** How often the world's data (tile map, entities, etc) should be saved,
        in seconds. */
    static constexpr float SAVE_PERIOD_S{60 * 15};

    /** How long it takes a client's entity to leave the world, in seconds.
        Applies to both ways of leaving:
          - Explicit logout: The client requests a logout and its entity
            stays in the world during the countdown. If the entity moves or
            casts, the logout is canceled. When the countdown finishes, the
            entity is removed immediately.
          - Disconnect: The client disconnects without logging out and its
            entity lingers for this long before being removed. If the same
            account logs in during this time, it takes control of the
            existing entity.
            If an account logs in while it already has a connected client,
            the old client is disconnected and this delay applies as usual. */
    static constexpr double LOGOUT_DELAY_S{20};
    static constexpr unsigned int LOGOUT_DELAY_TICKS{static_cast<unsigned int>(
        LOGOUT_DELAY_S / SharedConfig::SIM_TICK_TIMESTEP_S)};

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
    static constexpr double CLIENT_TIMEOUT_S{4};

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

    //-------------------------------------------------------------------------
    // Chat
    //-------------------------------------------------------------------------
    /** How often we send client entity positions to the chat server, in
        seconds.
        The chat server uses these positions to route local chat, so a client
        whose entity moves may briefly hear (or miss) messages based on where
        its entity was. */
    static constexpr double CHAT_POSITION_UPDATE_INTERVAL_S{1};
    static constexpr unsigned int CHAT_POSITION_UPDATE_INTERVAL_TICKS{
        static_cast<unsigned int>(CHAT_POSITION_UPDATE_INTERVAL_S
                                  / SharedConfig::SIM_TICK_TIMESTEP_S)};
};

} // End namespace WorldServer
} // End namespace AM
