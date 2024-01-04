#pragma once

#include "Timer.h"

namespace AM
{
namespace Server
{

class World;

/**
 * Periodically saves the world's data:
 *   Tile map data is saved to TileMap.bin.
 *   Non-client entity data is saved to the database.
 *   Item definitions are saved to the database.
 */
class SaveSystem
{
public:
    SaveSystem(World& inWorld);

    /**
     * If a category data is due for saving, saves it.
     *
     * Configure through Config::SAVE_PERIOD.
     */
    void saveIfNecessary();

private:
    World& world;

    /** Used to track how much time has passed since the last save. */
    Timer saveTimer;

    /** These track whether a given type of data has been saved or not on this 
        iteration of the timer. */
    bool mapHasBeenSaved;
    bool nceHaveBeenSaved;
};

} // namespace Server
} // namespace AM
