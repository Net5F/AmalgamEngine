#include "SaveSystem.h"
#include "World.h"
#include "Config.h"

namespace AM
{
namespace Server
{

SaveSystem::SaveSystem(World& inWorld)
: world{inWorld}
, saveTimer{}
, mapHasBeenSaved{}
, nceHaveBeenSaved{}
{
}

void SaveSystem::saveIfNecessary()
{
    // We stagger the save times so that they don't cause a performance spike.
    static constexpr float MAP_SAVE_TIME_S{Config::SAVE_PERIOD_S * (1.0/3.0)};
    static constexpr float NCE_SAVE_TIME_S{Config::SAVE_PERIOD_S * (2.0/3.0)};
    static constexpr float ITEM_SAVE_TIME_S{Config::SAVE_PERIOD_S * (3.0/3.0)};

    // If enough time has passed, save the map state to TileMap.bin.
    if (!mapHasBeenSaved && (saveTimer.getTime() >= MAP_SAVE_TIME_S)) {
        world.tileMap.save("TileMap.bin");

        mapHasBeenSaved = true;
    }

    // If enough time has passed, save the non-client entity state to the 
    // database.
    if (!nceHaveBeenSaved && (saveTimer.getTime() >= NCE_SAVE_TIME_S)) {

        nceHaveBeenSaved = true;
    }

    // If enough time has passed, save the item definitions to the database.
    if (saveTimer.getTime() >= ITEM_SAVE_TIME_S) {

        saveTimer.reset();
    }
}

} // namespace Server
} // namespace AM
