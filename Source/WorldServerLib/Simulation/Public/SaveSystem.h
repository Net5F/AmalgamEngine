#pragma once

#include "ItemID.h"
#include "Timer.h"
#include "BinaryBuffer.h"
#include <vector>

namespace AM
{
namespace WorldServer
{
struct SimulationContext;
class World;
class ItemData;

/**
 * Periodically saves the world's data:
 *   Tile map data is saved to TileMap.bin.
 *   Non-player entity data is saved to the database.
 *   Player entity PlayerPersistence::World data is saved to the database,
 *   keyed by account ID. PlayerPersistence::Account data is queued to be
 *   saved to the account server (see AccountSaveQueue).
 *   Item data is saved to the database.
 *
 * After saving, the in-memory database is backed up to the file database. If
 * the backup fails, it's retried after BACKUP_RETRY_DELAY_S (instead of
 * waiting for the next save period). Items that fail to save are retried on
 * the next save.
 *
 * Note: Player entities are also saved when they're removed from the world
 *       (see ClientConnectionSystem).
 */
class SaveSystem
{
public:
    SaveSystem(const SimulationContext& inSimContext);

    /**
     * If data is due for saving, saves it. If a failed backup is due to be
     * retried, retries it.
     *
     * Configure through Config::SAVE_PERIOD.
     */
    void saveIfNecessary();

    /**
     * Saves all data and backs it up to the file database, blocking until the
     * backup finishes. If the backup fails, retries it up to
     * SHUTDOWN_BACKUP_ATTEMPTS times.
     *
     * Call this once when the server is shutting down, after the last sim
     * tick.
     */
    void saveForShutdown();

private:
    /** How long to wait before retrying a failed backup. */
    static constexpr double BACKUP_RETRY_DELAY_S{10};

    /** How many times saveForShutdown() will attempt the backup. */
    static constexpr int SHUTDOWN_BACKUP_ATTEMPTS{3};

    /** How long saveForShutdown() waits between backup attempts. */
    static constexpr double SHUTDOWN_BACKUP_RETRY_DELAY_S{1};

    /**
     * Saves all data to the in-memory database. Player entity Account data
     * is queued to be saved to the account server.
     */
    void saveAll();

    /**
     * Starts backing up the in-memory database to the file database.
     */
    void startBackup();

    /**
     * If a backup that we started has finished, checks its result. If it
     * failed, schedules a retry.
     */
    void checkBackupResult();

    /**
     * Adds the given item to updatedItems.
     */
    void itemUpdated(ItemID itemID);

    /**
     * Saves items to the in-memory database.
     */
    void saveItems();

    /**
     * Saves World::globalStoredValueMap to the in-memory database.
     */
    void saveGlobalStoredValues();

    World& world;
    ItemData& itemData;

    /** Holds a history of items that have been updated.
        Used to know which items need to be saved. */
    std::vector<ItemID> updatedItems;

    /** Used to track how much time has passed since the last save. */
    Timer saveTimer;

    /** Used to track how much time has passed since a backup failed. */
    Timer backupRetryTimer;

    /** If true, we started a backup and haven't checked its result yet. */
    bool awaitingBackupResult;

    /** If true, a backup failed and should be retried once backupRetryTimer
        reaches BACKUP_RETRY_DELAY_S. */
    bool backupRetryPending;

    /** Scratch buffer used while serializing data. */
    BinaryBuffer workBuffer;
};

} // namespace WorldServer
} // namespace AM
