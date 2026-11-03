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
#include <chrono>
#include <thread>

namespace AM
{
namespace WorldServer
{

SaveSystem::SaveSystem(const SimulationContext& inSimContext)
: world{inSimContext.simulation.getWorld()}
, itemData{inSimContext.itemData}
, updatedItems{}
, saveTimer{}
, backupRetryTimer{}
, awaitingBackupResult{false}
, backupRetryPending{false}
, workBuffer{}
{
    // When an item is created or updated, add it to updatedItems.
    itemData.itemCreated.connect<&SaveSystem::itemUpdated>(this);
    itemData.itemUpdated.connect<&SaveSystem::itemUpdated>(this);
}

void SaveSystem::saveIfNecessary()
{
    // If a backup that we started has finished, check whether it failed.
    checkBackupResult();

    // If enough time has passed and a backup isn't still underway, save
    // everything to the database and back it up.
    // Note: This also takes care of any pending backup retry.
    bool backupInProgress{world.database->backupIsInProgress()};
    if ((saveTimer.getTime() >= Config::SAVE_PERIOD_S) && !backupInProgress) {
        saveAll();
        startBackup();
        saveTimer.reset();
    }
    // If a failed backup is due to be retried, retry it.
    // Note: The in-memory database still has all of our data, so we only need
    //       to back up again.
    else if (backupRetryPending
             && (backupRetryTimer.getTime() >= BACKUP_RETRY_DELAY_S)
             && !backupInProgress) {
        LOG_INFO("Retrying database backup...");
        startBackup();
    }
}

void SaveSystem::saveForShutdown()
{
    // If a periodic backup is still running, wait for it. We can't start
    // another until it finishes.
    world.database->waitForBackup();

    saveAll();

    // Backup the in-memory database to the file database, and wait for it to
    // finish. If it fails, retry a few times.
    for (int attempt{1}; attempt <= SHUTDOWN_BACKUP_ATTEMPTS; ++attempt) {
        LOG_INFO("Backing up database...");
        world.database->backupToFile();
        world.database->waitForBackup();
        if (world.database->lastBackupSucceeded()) {
            return;
        }

        if (attempt < SHUTDOWN_BACKUP_ATTEMPTS) {
            LOG_WARN("Database backup failed. Retrying in %.0fs.",
                     SHUTDOWN_BACKUP_RETRY_DELAY_S);
            std::this_thread::sleep_for(
                std::chrono::duration<double>(SHUTDOWN_BACKUP_RETRY_DELAY_S));
        }
    }

    LOG_WARN("Database backup failed %d times during shutdown. Data since "
             "the last successful backup was lost.",
             SHUTDOWN_BACKUP_ATTEMPTS);
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

void SaveSystem::startBackup()
{
    world.database->backupToFile();
    awaitingBackupResult = true;
    backupRetryPending = false;
}

void SaveSystem::checkBackupResult()
{
    if (!awaitingBackupResult || world.database->backupIsInProgress()) {
        return;
    }
    awaitingBackupResult = false;

    if (!(world.database->lastBackupSucceeded())) {
        LOG_WARN("Database backup failed. Retrying in %.0fs. Data since the "
                 "last successful backup will be lost if the server exits "
                 "before a backup succeeds.",
                 BACKUP_RETRY_DELAY_S);
        backupRetryPending = true;
        backupRetryTimer.reset();
    }
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
    // Note: We keep any items that fail to save, so they're retried next time.
    std::erase_if(updatedItems, [&](ItemID itemID) {
        const Item* updatedItem{itemData.getItem(itemID)};

        workBuffer.clear();
        workBuffer.resize(Serialize::measureSize(*updatedItem));
        Serialize::toBuffer(workBuffer.data(), workBuffer.size(), *updatedItem);

        return world.database->saveItemData(
            updatedItem->numericID, workBuffer,
            itemData.getItemVersion(updatedItem->numericID),
            itemData.getItemInitScript(updatedItem->numericID).script);
    });

    if (!(updatedItems.empty())) {
        LOG_ERROR("%zu items failed to save. They will be retried, but will "
                  "be lost if the server exits first.",
                  updatedItems.size());
    }
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
