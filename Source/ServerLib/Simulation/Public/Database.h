#pragma once

#include "ItemID.h"
#include "SQLiteCpp/SQLiteCpp.h"
#include "entt/fwd.hpp"
#include <SDL_stdinc.h>
#include <optional>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <span>

namespace AM
{
namespace Server
{
class World;

/**
 * Interface for interacting with the database.
 * 
 * We use the database to persist item definitions, non-client entity data, 
 * and tile map data as blobs.
 *
 * To avoid blocking the main loop, we first copy all of our data into an 
 * in-memory database. Then, we use a separate thread to backup the in-memory 
 * database to a file. See SaveSystem.h for more info.
 *
 * Note: Client entity data is persisted in the account database, not here.
 */
class Database
{
public:
    Database();

    ~Database();

    /**
     * Begins a transaction. While a transaction is ongoing, queries will be 
     * queued until commitTransaction() is called.
     */
    void startTransaction();

    /**
     * If a transaction is ongoing, commits it. This will execute all queued 
     * queries.
     */
    void commitTransaction();

    /**
     * Backs up our in-memory database to the file database.
     */
    void backupToFile();

    /**
     * @return true if a backup is currently being performed, else false.
     */
    bool backupIsInProgress();

    //-------------------------------------------------------------------------
    // Entities
    //-------------------------------------------------------------------------
    /**
     * Adds or overwrites an entity table entry.
     *
     * @param entity The entity entry to update.
     * @param engineComponentData A serialized 
     *                            std::vector<EnginePersistedComponent>.
     * @param projectComponentData A serialized 
     *                             std::vector<ProjectPersistedComponent>.
     */
    void saveEntityData(entt::entity entity,
                        std::span<const Uint8> engineComponentData,
                        std::span<const Uint8> projectComponentData);

    /**
     * Attempts to delete an entity table entry for the given entity.
     *
     * If the entity is not found in the database, does nothing.
     */
    void deleteEntityData(entt::entity entity);

    /**
     * Calls the given callback on each entity data entry.
     *
     * @param callback A callback of form void(entt::entity, 
     *                 std::span<const Uint8>, std::span<const Uint8>) that 
     *                 expects the entity's ID, a serialized 
     *                 std::vector<EnginePersistedComponent>, and a serialized 
     *                 std::vector<ProjectPersistedComponent>.
     */
    template<typename Func>
    void iterateEntities(Func callback)
    {
        while (iterateEntitiesQuery->executeStep()) {
            SQLite::Column engineComponentDataColumn{
                iterateEntitiesQuery->getColumn(1)};
            SQLite::Column projectComponentDataColumn{
                iterateEntitiesQuery->getColumn(2)};
            callback(static_cast<entt::entity>(
                         iterateEntitiesQuery->getColumn(0).getInt()),
                     std::span<const Uint8>{
                         static_cast<const Uint8*>(
                             engineComponentDataColumn.getBlob()),
                         static_cast<std::size_t>(
                             engineComponentDataColumn.getBytes())},
                     std::span<const Uint8>{
                         static_cast<const Uint8*>(
                             projectComponentDataColumn.getBlob()),
                         static_cast<std::size_t>(
                             projectComponentDataColumn.getBytes())});
        }
        iterateEntitiesQuery->reset();
    }

    //-------------------------------------------------------------------------
    // Items
    //-------------------------------------------------------------------------
    /**
     * Adds or overwrites an item table entry.
     *
     * @param itemID The item entry to update.
     * @param itemData A serialized Item struct.
     */
    void saveItemData(ItemID itemID, std::span<const Uint8> itemData);

    /**
     * Attempts to delete an item table entry for the given item.
     *
     * If the item is not found in the database, does nothing.
     */
    void deleteItemData(ItemID itemID);

    /**
     * Calls the given callback on each item data entry.
     *
     * @param callback A callback of form void(ItemID, const Uint8*,
     *                 std::size_t) that expects the item's ID, and a 
     *                 serialized Item struct.
     */
    template<typename Func>
    void iterateItems(Func callback)
    {
        while (iterateItemsQuery->executeStep()) {
            SQLite::Column dataColumn{iterateItemsQuery->getColumn(1)};
            callback(static_cast<ItemID>(
                         iterateItemsQuery->getColumn(0).getInt()),
                     static_cast<const Uint8*>(dataColumn.getBlob()),
                     dataColumn.getBytes());
        }
        iterateItemsQuery->reset();
    }

    //-------------------------------------------------------------------------
    // Stored Values
    //-------------------------------------------------------------------------
    /**
     * Adds or overwrites the entity stored value ID map entry.
     *
     * @param entityStoredValueIDMapData A serialized 
     *                                   World::entityStoredValueIDMap.
     */
    void saveEntityStoredValueIDMap(
        std::span<const Uint8> entityStoredValueIDMapData);

    /**
     * Calls the given callback on the entity stored value ID map data entry.
     *
     * @param callback A callback of form void(const Uint8*, std::size_t) that 
     *                 expects a serialized entity stored value ID map.
     */
    template<typename Func>
    void getEntityStoredValueIDMap(Func callback)
    {
        getEntityStoredValueIDMapQuery->executeStep();
        SQLite::Column dataColumn{getEntityStoredValueIDMapQuery->getColumn(0)};
        callback(static_cast<const Uint8*>(dataColumn.getBlob()),
                 dataColumn.getBytes());

        getEntityStoredValueIDMapQuery->reset();
    }

    /**
     * Adds or overwrites the entity stored value ID map entry.
     *
     * @param globalStoredValueMapData A serialized World::globalStoredValueMap.
     */
    void saveGlobalStoredValueMap(
        std::span<const Uint8> globalStoredValueMapData);

    /**
     * Calls the given callback on the entity stored value ID map data entry.
     *
     * @param callback A callback of form void(const Uint8*, std::size_t) that 
     *                 expects a serialized entity stored value ID map.
     */
    template<typename Func>
    void getGlobalStoredValueMap(Func callback)
    {
        getGlobalStoredValueMapQuery->executeStep();
        SQLite::Column dataColumn{getGlobalStoredValueMapQuery->getColumn(0)};
        callback(static_cast<const Uint8*>(dataColumn.getBlob()),
                 dataColumn.getBytes());

        getGlobalStoredValueMapQuery->reset();
    }

protected:
    /**
     * Creates our tables in Database.db3, if they don't already exist.
     */
    void initTables();

    /**
     * Compares each version number from the database to the current version 
     * numbers in code. If the data version doesn't match the code version, 
     * prints an appropriate error and exits.
     */
    void checkDataVersions();

    /**
     * Thread function.
     * Waits for backupToFile() to flag that a backup should begin.
     * 
     * Backs up the in-memory database to the file database.
     */
    void performBackup();

    /** In-memory database. Used to gather our data so we can safely work 
        with it in another thread. */
    SQLite::Database database;

    /** File-backed database. Used to persist our data to a file. */
    SQLite::Database backupDatabase;

    /** If valid, this is the current ongoing transaction. */
    std::optional<SQLite::Transaction> currentTransaction;

    /** Calls performBackup(). */
    std::thread backupThreadObj;
    /** Turn false to signal that the send and receive threads should end. */
    std::atomic<bool> exitRequested;

    /** Used for signaling the backup thread. */
    std::mutex backupMutex;
    std::condition_variable_any backupCondVar;
    // Note: This doesn't need to be atomic to work with the condition variable,
    //       but it needs to be atomic to return it from backupIsInProgress().
    std::atomic<bool> backupRequested;

    // Pre-built queries
    std::unique_ptr<SQLite::Statement> insertEntityQuery;
    std::unique_ptr<SQLite::Statement> deleteEntityQuery;
    std::unique_ptr<SQLite::Statement> iterateEntitiesQuery;
    std::unique_ptr<SQLite::Statement> insertItemQuery;
    std::unique_ptr<SQLite::Statement> deleteItemQuery;
    std::unique_ptr<SQLite::Statement> iterateItemsQuery;
    std::unique_ptr<SQLite::Statement> insertEntityStoredValueIDMapQuery;
    std::unique_ptr<SQLite::Statement> getEntityStoredValueIDMapQuery;
    std::unique_ptr<SQLite::Statement> insertGlobalStoredValueMapQuery;
    std::unique_ptr<SQLite::Statement> getGlobalStoredValueMapQuery;
};

} // namespace Server
} // namespace AM
