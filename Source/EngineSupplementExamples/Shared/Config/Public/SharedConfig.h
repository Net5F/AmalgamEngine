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
    static constexpr std::size_t MAX_ENTITIES{200};

    /** The max number of interactions that an entity can support. */
    static constexpr std::size_t MAX_ENTITY_INTERACTIONS{64};

    /** The max number of non-built-in interactions that an item can support. */
    static constexpr std::size_t MAX_ITEM_CUSTOM_INTERACTIONS{64};

    /** The x and y axis width, in world units, of our tiles. */
    static constexpr unsigned int TILE_WORLD_WIDTH{32};

    /** The x and y axis width, in tiles, of our chunks. */
    static constexpr unsigned int CHUNK_WIDTH{16};

    /** The number of tiles in a chunk. */
    static constexpr unsigned int CHUNK_TILE_COUNT{CHUNK_WIDTH * CHUNK_WIDTH};

    /** The x and y axis width, in tiles, of a cell in our spatial
        partitioning grid.
        Note: The map's size must be evenly divisible by this number. */
    static constexpr unsigned int CELL_WIDTH{4};

    /** The number of world units around an entity that are considered to be
        within the entity's "Area of Interest".
        Used in the simulation to tell if data is relevant to a client. */
    static constexpr float AOI_RADIUS{TILE_WORLD_WIDTH * 8};

    /** How close you need to be to an entity, in world units, to interact
        with it. */
    static constexpr float INTERACTION_DISTANCE{TILE_WORLD_WIDTH * 1};
    static constexpr float SQUARED_INTERACTION_DISTANCE{INTERACTION_DISTANCE
                                                        * INTERACTION_DISTANCE};

    /** The string ID of the default graphic set for entities. */
    static constexpr const char* DEFAULT_ENTITY_GRAPHIC_SET{"ghost"};

    /** The speed that entities will move at. Eventually we'll support
        dynamically setting this based on stats, for now it's a constant. */
    static constexpr float MOVEMENT_VELOCITY{50};

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
    /** The width of a tile in screen coordinates. */
    static constexpr unsigned int TILE_SCREEN_WIDTH{256};
    /** The height of the top face of a tile in screen coordinates. */
    static constexpr unsigned int TILE_SCREEN_HEIGHT{128};

    /** The total width of a standard tile sprite. */
    static constexpr unsigned int TILE_SPRITE_WIDTH{256};
    /** The total height of a standard tile sprite. */
    static constexpr unsigned int TILE_SPRITE_HEIGHT{512};

    // Note: This solution is temporary. Eventually, the Tile concept will be
    //       replaced with 3D Blocks and this value will be derivable like the
    //       others.
    /** The value used to scale world Z-axis units to screen Y-axis units. */
    static constexpr float Z_SCREEN_SCALE{2.f};

    /** The number of world units around the player that are considered to be
        within the player's view range.
        Used to determine how much of the world should be rendered. */
    static constexpr float VIEW_RADIUS{TILE_WORLD_WIDTH * 8};
};

} // End namespace AM
