#pragma once

#include <cstddef>

namespace AM
{
/**
 * Contains shared configuration constants that must be consistent between the
 * client, server, and resource importer.
 *
 * Note: When you update the engine, you may need to add additional fields
 *       here. Refer to the engine's default SharedConfig.h.
 */
class SharedConfig
{
public:
    //-------------------------------------------------------------------------
    // General Simulation
    //-------------------------------------------------------------------------
    /** The rate at which the simulation will be processed. */
    static constexpr unsigned int SIM_TICKS_PER_SECOND{30};
    static constexpr double SIM_TICK_TIMESTEP_S{
        1.0 / static_cast<double>(SIM_TICKS_PER_SECOND)};

    /** The max number of entities that we will allow. */
    static constexpr std::size_t MAX_ENTITIES{1100};

    /** The string ID of the default graphic set for entities. */
    static constexpr const char* DEFAULT_ENTITY_GRAPHIC_SET{"ghost"};

    //-------------------------------------------------------------------------
    // Movement
    //-------------------------------------------------------------------------
    /** The force of gravity in world units per second. */
    static constexpr float FORCE_OF_GRAVITY{20};

    /** The maximum negative Z velocity that entities can reach. */
    static constexpr float TERMINAL_VELOCITY{-300};

    /** The amount of velocity that's added to the player if they try to move
        while falling straight through the air, e.g. after jumping straight up.
        This is a constant instead of being affected by runspeed, because it's
        intended to be a consistent, small movement. */
    static constexpr float VERTICAL_FALL_MOVE_VELOCITY{15};

    //-------------------------------------------------------------------------
    // Interactions
    //-------------------------------------------------------------------------
    /** The max number of interactions that an entity can support.
        Interactions are the options that show up when you right-click an
        entity or item. */
    static constexpr std::size_t MAX_ENTITY_INTERACTIONS{64};

    /** The max number of non-built-in interactions that an item can support. */
    static constexpr std::size_t MAX_ITEM_CUSTOM_INTERACTIONS{64};

    //-------------------------------------------------------------------------
    // Tile Map
    //-------------------------------------------------------------------------
    /** The maximum map size that we'll load, in tiles. */
    static constexpr std::size_t MAX_MAP_WIDTH_TILES{1'024};

    /** The x and y axis width, in world units, of our tiles. */
    static constexpr std::size_t TILE_WORLD_WIDTH{32};

    /** The z axis height, in world units, of our tiles. */
    static constexpr std::size_t TILE_WORLD_HEIGHT{84};

    /** The x and y axis width, in tiles, of our chunks.
        Note: This doesn't apply to the z axis. Chunks are always 1 tile tall.
     */
    static constexpr std::size_t CHUNK_WIDTH{16};

    /** The number of tiles in a chunk. */
    static constexpr std::size_t CHUNK_TILE_COUNT{CHUNK_WIDTH * CHUNK_WIDTH};

    /** The x and y axis width, in tiles, of a cell in our entity spatial
        partitioning grid (EntityLocator). */
    static constexpr std::size_t ENTITY_LOCATOR_CELL_WIDTH{4};

    /** The z axis width, in tiles, of a cell in our entity spatial partitioning
        grid (EntityLocator). */
    static constexpr std::size_t ENTITY_LOCATOR_CELL_HEIGHT{2};

    /** The x and y axis width, in tiles, of a cell in our collision spatial
        partitioning grid (CollisionLocator). */
    static constexpr std::size_t COLLISION_LOCATOR_CELL_WIDTH{4};

    /** The z axis width, in tiles, of a cell in our collision spatial
        partitioning grid (CollisionLocator). */
    static constexpr std::size_t COLLISION_LOCATOR_CELL_HEIGHT{2};

    //-------------------------------------------------------------------------
    // Entity Replication
    //-------------------------------------------------------------------------
    /** The number of world units around an entity that are considered to be
        within the entity's "Area of Interest" cylinder.
        Used in the simulation to tell if data is relevant to a client. */
    static constexpr float AOI_RADIUS{TILE_WORLD_WIDTH * 8};

    /** The number of world units above and below an entity that are considered
        to be within the entity's "Area of Interest" cylinder. */
    static constexpr float AOI_HALF_HEIGHT{TILE_WORLD_HEIGHT * 3};

    //-------------------------------------------------------------------------
    // Chat
    //-------------------------------------------------------------------------
    /** The number of world units around an entity that are considered to be
        within range of its local chat messages. */
    static constexpr float LOCAL_CHAT_RADIUS{AOI_RADIUS};

    /** The number of world units above and below an entity that are considered
        to be within range of its local chat messages. */
    static constexpr float LOCAL_CHAT_HALF_HEIGHT{AOI_HALF_HEIGHT};

    //-------------------------------------------------------------------------
    // Casting
    //-------------------------------------------------------------------------
    /** The distance above a casting target circle that will be included in
        the area of effect. */
    static constexpr float CAST_TARGET_CYLINDER_HEIGHT{3};
    static constexpr float CAST_TARGET_CYLINDER_HALF_HEIGHT{
        CAST_TARGET_CYLINDER_HEIGHT / 2.0};

    /** The length of the global cooldown for casting. */
    static constexpr double CAST_GLOBAL_COOLDOWN_S{1.0};
    static constexpr unsigned int CAST_GLOBAL_COOLDOWN_TICKS{
        static_cast<unsigned int>(CAST_GLOBAL_COOLDOWN_S
                                  / SIM_TICK_TIMESTEP_S)};

    /** The standard range for entity interactions, such as "Talk".
        This is the max distance between the caster and the target entity.
        This will be the range of all engine-provided entity interactions, and
        you can choose to use it for project interactions as well. */
    static constexpr float CAST_ENTITY_INTERACTION_STANDARD_RANGE{
        TILE_WORLD_WIDTH};

    //-------------------------------------------------------------------------
    // Pathfinding
    //-------------------------------------------------------------------------
    // Note: Pathfinding measures heights in "thirds" of a tile
    //       (TILE_WORLD_HEIGHT / 3), matching our terrain heights.

    /** The width, in tiles, of the common pathfinding agent's footprint.
        Pathfinding is optimized for agents that fit the common agent profile.
        Other agents can still path, but some optimizations won't apply to
        them. */
    static constexpr unsigned int PATHFINDING_COMMON_FOOTPRINT_TILES{1};

    /** The height, in thirds, of the common pathfinding agent. */
    static constexpr unsigned int PATHFINDING_COMMON_HEIGHT_THIRDS{2};

    /** The max height, in thirds, that the common pathfinding agent can climb
        when moving from one tile to the next. */
    static constexpr unsigned int PATHFINDING_COMMON_CLIMB_THIRDS{1};

    /** The max height, in thirds, that the common pathfinding agent will drop
        when moving from one tile to the next. */
    static constexpr unsigned int PATHFINDING_COMMON_MAX_DROP_THIRDS{3};

    /** The extra cost of a step that requires a jump, relative to walking
        one tile. Raising this makes agents prefer walking around ledges
        instead of jumping onto them. */
    static constexpr float PATHFINDING_JUMP_COST{1.f};

    /** The max number of nodes that a single path search will expand before
        giving up. 4096 covers roughly a 64x64 tile area, which is plenty for
        combat. Searches that need more (e.g. returning to a distant spawn)
        can pass a higher limit. */
    static constexpr std::size_t PATHFINDING_MAX_SEARCH_NODES{4096};

    /** The max number of nodes that path searches will expand per tick,
        across all queued path requests. Searches that don't finish continue
        on the next tick.
        At roughly 0.5us per node, 2048 is about 1ms per tick. */
    static constexpr std::size_t PATHFINDING_NODES_PER_TICK{2048};

    /** Agents that are following a target entity will find a new path when
        the target moves this many tiles away from where their current path
        ends. */
    static constexpr float PATHFINDING_REPATH_DISTANCE_TILES{2};

    /** The min number of ticks between an agent's path requests. Keeps
        agents with moving targets from using up the per-tick node budget. */
    static constexpr unsigned int PATHFINDING_REPATH_INTERVAL_TICKS{15};

    /** If an agent moves less than a quarter of a tile within this many ticks
        while following a path, it's considered stuck. It'll try jumping,
        then finding a new path, then give up. */
    static constexpr unsigned int PATHFINDING_STUCK_TICKS{30};

    //-------------------------------------------------------------------------
    // Network
    //-------------------------------------------------------------------------
    /** The rate at which the server will batch and send queued messages. */
    static constexpr unsigned int SERVER_NETWORK_TICKS_PER_SECOND{20};
    static constexpr double SERVER_NETWORK_TICK_TIMESTEP_S{
        1.0 / static_cast<double>(SERVER_NETWORK_TICKS_PER_SECOND)};

    /** The rate at which the client will heartbeat (if necessary). */
    static constexpr unsigned int CLIENT_NETWORK_TICKS_PER_SECOND{1};
    static constexpr double CLIENT_NETWORK_TICK_TIMESTEP_S{
        1.0 / static_cast<double>(CLIENT_NETWORK_TICKS_PER_SECOND)};

    /** Message batches larger than this size (in bytes) will be compressed
        before sending. */
    static constexpr std::size_t BATCH_COMPRESSION_THRESHOLD{50};

    /** The max size that an uncompressed message batch can be.
        Used to allocate our message buffers.

        If it was free, we would just send the actual uncompressed size of
        each message batch over the wire. To save bytes, we instead configure
        this constant.
        Our message buffers are very small relative to available RAM, so there
        isn't much harm in raising this to be well above what we actually need.

        Note: Each simulated client in LoadTestClient instantiates a buffer,
              so you may need to be conscious of this size in that case. */
    static constexpr std::size_t MAX_BATCH_SIZE{20'000};

    //-------------------------------------------------------------------------
    // Renderer
    //-------------------------------------------------------------------------
    /** The width of the top face of a tile in screen coordinates. */
    static constexpr unsigned int TILE_FACE_SCREEN_WIDTH{256};
    /** The height of the top face of a tile in screen coordinates. */
    static constexpr unsigned int TILE_FACE_SCREEN_HEIGHT{128};
    /** The height of the side of a tile in screen coordinates, i.e. how tall
        the world-space z axis height of a tile should appear on screen.
        Since Z coordinate contribution to the screen Y axis is linear,
        this / TILE_WORLD_HEIGHT can be thought of as our "Z scaling factor" */
    static constexpr unsigned int TILE_SIDE_SCREEN_HEIGHT{168};

    /** The number of world units around the player that are considered to be
        within the player's view range.
        Used to determine how much of the world should be rendered. */
    static constexpr float VIEW_RADIUS{TILE_WORLD_WIDTH * 8};

    //-------------------------------------------------------------------------
    // Project
    //-------------------------------------------------------------------------
    /** If true, build mode will only be allowed in the "build area".
        If false, clients can make changes anywhere in the world. */
    static constexpr bool RESTRICT_WORLD_CHANGES{false};
};

} // End namespace AM
