#pragma once

#include <cstddef>

namespace AM
{
/**
 * Contains shared configuration constants that must be consistent between the
 * client, server, and resource importer.
 */
class SharedConfig
{
public:
    //-------------------------------------------------------------------------
    // Simulation
    //-------------------------------------------------------------------------
    /** The rate at which the simulation will be processed. */
    static constexpr unsigned int SIM_TICKS_PER_SECOND{30};
    static constexpr double SIM_TICK_TIMESTEP_S{
        1.0 / static_cast<double>(SIM_TICKS_PER_SECOND)};

    /** The max number of entities that we will allow. */
    static constexpr std::size_t MAX_ENTITIES{1100};

    /** The max number of interactions that an entity can support.
        Interactions are the options that show up when you right-click an 
        entity or item. */
    static constexpr std::size_t MAX_ENTITY_INTERACTIONS{64};

    /** The max number of non-built-in interactions that an item can support. */
    static constexpr std::size_t MAX_ITEM_CUSTOM_INTERACTIONS{64};

    /** The maximum map size that we'll load, in tiles. */
    static constexpr std::size_t MAX_MAP_WIDTH_TILES{1'000};

    /** The x and y axis width, in world units, of our tiles. */
    static constexpr std::size_t TILE_WORLD_WIDTH{32};

    /** The z axis height, in world units, of our tiles. */
    static constexpr std::size_t TILE_WORLD_HEIGHT{84};

    /** The x and y axis width, in tiles, of our chunks.
        Note: This doesn't apply to the z axis. Chunks are always 1 tile tall. */
    static constexpr std::size_t CHUNK_WIDTH{16};

    /** The number of tiles in a chunk. */
    static constexpr std::size_t CHUNK_TILE_COUNT{CHUNK_WIDTH * CHUNK_WIDTH};

    /** The x and y axis width, in tiles, of a cell in our entity spatial
        partitioning grid (EntityLocator). */
    static constexpr std::size_t ENTITY_LOCATOR_CELL_WIDTH{4};

    /** The z axis width, in tiles, of a cell in our entity spatial partitioning 
        grid (EntityLocator). */
    static constexpr std::size_t ENTITY_LOCATOR_CELL_HEIGHT{2};

    /** The number of world units around an entity that are considered to be
        within the entity's "Area of Interest" cylinder.
        Used in the simulation to tell if data is relevant to a client. */
    static constexpr float AOI_RADIUS{TILE_WORLD_WIDTH * 8};

    /** The number of world units above and below an entity that are considered
        to be within the entity's "Area of Interest" cylinder. */
    static constexpr float AOI_HALF_HEIGHT{TILE_WORLD_HEIGHT * 3};

    /** How close you need to be to an entity, in world units, to interact
        with it. */
    static constexpr float INTERACTION_DISTANCE{TILE_WORLD_WIDTH * 1};
    static constexpr float SQUARED_INTERACTION_DISTANCE{INTERACTION_DISTANCE
                                                        * INTERACTION_DISTANCE};

    /** The string ID of the default graphic set for entities. */
    static constexpr const char* DEFAULT_ENTITY_GRAPHIC_SET{"ghost"};

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

    /** The total width of a standard tile sprite. */
    static constexpr unsigned int TILE_SPRITE_WIDTH{256};
    /** The total height of a standard tile sprite. */
    static constexpr unsigned int TILE_SPRITE_HEIGHT{512};

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
