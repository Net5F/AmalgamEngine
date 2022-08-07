#pragma once

#include "Timer.h"

namespace AM
{
namespace Server
{

class World;

/**
 * Periodically saves the world's tile map to TileMap.bin.
 */
class MapSaveSystem
{
public:
    MapSaveSystem(World& inWorld);

    /**
     * If enough time has passed, saves the tile map.
     *
     * Configure through Config::MAP_SAVE_PERIOD.
     */
    void saveMapIfNecessary();

private:
    World& world;

    /** Used to track how much time has passed since the last save. */
    Timer saveTimer;
};

} // namespace Server
} // namespace AM
