#pragma once

namespace AM
{
/**
 * This class contains shared configuration that should be consistent between
 * the server and client.
 */
class SharedConfig
{
public:
    //-------------------------------------------------------------------------
    // Simulation
    //-------------------------------------------------------------------------
    /** The rate at which the simulation will be processed. */
    static constexpr unsigned int SIM_TICKS_PER_SECOND = 30;
    static constexpr double SIM_TICK_TIMESTEP_S
        = 1.0 / static_cast<double>(SIM_TICKS_PER_SECOND);

    /** The max number of entities that we will allow. */
    static constexpr unsigned int MAX_ENTITIES = 1010;

    // TODO: This can get moved to the server config when AoI gets improved.
    /** The distance in world coordinates that we add to a player's AoI to let
        peers load before they're in view. */
    static constexpr unsigned int AOI_BUFFER_DISTANCE = 700;

    /** The width of the world tiles in world coordinates. */
    static constexpr unsigned int TILE_WORLD_WIDTH = 32;
    /** The height of the world tiles in world coordinates. */
    static constexpr unsigned int TILE_WORLD_HEIGHT = 32;

    /** The width of the world's terrain map in tiles. */
    static constexpr unsigned int WORLD_WIDTH = 10;
    /** The height of the world's terrain map in tiles. */
    static constexpr unsigned int WORLD_HEIGHT = 10;

    //-------------------------------------------------------------------------
    // Network
    //-------------------------------------------------------------------------
    /** The rate at which the network will send queued messages, or a heartbeat
        if no messages are queued. */
    static constexpr unsigned int NETWORK_TICKS_PER_SECOND = 20;
    static constexpr double NETWORK_TICK_TIMESTEP_S
        = 1.0 / static_cast<double>(NETWORK_TICKS_PER_SECOND);

    //-------------------------------------------------------------------------
    // Renderer
    //-------------------------------------------------------------------------
    // TODO: These should be in the client config only, but the server uses
    //       them for AoI at the moment.
    static constexpr unsigned int SCREEN_WIDTH = 1280;
    static constexpr unsigned int SCREEN_HEIGHT = 720;

    // TODO: These are shared between client/editor because
    //       TransformationHelpers uses them. Should they be dynamically
    //       passed in or something?
    /** The width of a tile in screen coordinates. */
    static constexpr unsigned int TILE_SCREEN_WIDTH = 256;
    /** The height of the top face of a tile in screen coordinates. */
    static constexpr unsigned int TILE_SCREEN_HEIGHT = 128;
    /** The height of the edge of a tile in screen coordinates. */
    static constexpr unsigned int TILE_SCREEN_EDGE_HEIGHT = 10;

    /** The total width of a standard tile sprite. */
    static constexpr unsigned int TILE_SPRITE_WIDTH = 256;
    /** The total height of a standard tile sprite. */
    static constexpr unsigned int TILE_SPRITE_HEIGHT = 512;

    // Note: This solution is temporary. Eventually, the Tile concept will be
    //       replaced with 3D Blocks and this value will be derivable like the
    //       others.
    /** The value used to scale world Z-axis units to screen Y-axis units. */
    static constexpr float Z_SCREEN_SCALE = 2.f;
};

} // End namespace AM
