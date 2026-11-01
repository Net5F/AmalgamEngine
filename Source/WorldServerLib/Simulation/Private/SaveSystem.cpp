#include "SaveSystem.h"
#include "SimulationContext.h"
#include "Simulation.h"
#include "World.h"
#include "ItemData.h"
#include "Config.h"
#include "Database.h"
#include "Serialize.h"
#include "Log.h"
#include <algorithm>

namespace AM
{
namespace WorldServer
{

SaveSystem::SaveSystem(const SimulationContext& inSimContext)
: world{inSimContext.simulation.getWorld()}
, itemData{inSimContext.itemData}
, updatedItems{}
, saveTimer{}
, workBuffer{}
{
    // When an item is created or updated, add it to updatedItems.
    itemData.itemCreated.connect<&SaveSystem::itemUpdated>(this);
    itemData.itemUpdated.connect<&SaveSystem::itemUpdated>(this);
}

void SaveSystem::saveIfNecessary()
{
    // If enough time has passed and a backup isn't still underway, save
    // everything to the database.
    if ((saveTimer.getTime() >= Config::SAVE_PERIOD_S)
        && !(world.database->backupIsInProgress())) {
        saveAll();

        // Backup the in-memory database to the file database.
        world.database->backupToFile();

        saveTimer.reset();
    }
}

void SaveSystem::saveForShutdown()
{
    // If a periodic backup is still running, wait for it. We can't start
    // another until it finishes.
    world.database->waitForBackup();

    saveAll();

    // Backup the in-memory database to the file database, and wait for it to
    // finish.
    LOG_INFO("Backing up database...");
    world.database->backupToFile();
    world.database->waitForBackup();
}

void SaveSystem::saveAll()
{
    LOG_INFO("Saving entities, client entities, items, and map...");

    // Save all of our data to the in-memory database.
    SQLite::Transaction transaction{world.database->startTransaction()};

    world.saveHelper.saveNonClientEntities();
    world.saveHelper.saveClientEntities();
    saveItems();
    saveGlobalStoredValues();
    // TODO: Track changed tiles and save to the database.
    world.tileMap.save("TileMap.bin");

    transaction.commit();
}

void SaveSystem::itemUpdated(ItemID itemID)
{
    updatedItems.emplace_back(itemID);
}

void SaveSystem::saveItems()
{
    // Remove duplicates from the vector.
    std::sort(updatedItems.begin(), updatedItems.end());
    updatedItems.erase(std::unique(updatedItems.begin(), updatedItems.end()),
                       updatedItems.end());

    // Queue all of our item save queries.
    for (ItemID itemID : updatedItems) {
        const Item* updatedItem{itemData.getItem(itemID)};

        workBuffer.clear();
        workBuffer.resize(Serialize::measureSize(*updatedItem));
        Serialize::toBuffer(workBuffer.data(), workBuffer.size(), *updatedItem);

        world.database->saveItemData(
            updatedItem->numericID, workBuffer,
            itemData.getItemVersion(updatedItem->numericID),
            itemData.getItemInitScript(updatedItem->numericID).script);
    }

    updatedItems.clear();
}

void SaveSystem::saveGlobalStoredValues()
{
    // Serialize the global stored value map.
    workBuffer.clear();
    workBuffer.resize(Serialize::measureSize(world.globalStoredValueMap));
    Serialize::toBuffer(workBuffer.data(), workBuffer.size(),
                        world.globalStoredValueMap);

    // Queue the save query.
    world.database->saveGlobalStoredValueMap(workBuffer);
}

} // namespace WorldServer
} // namespace AM
