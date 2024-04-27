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
     * @param entityDataBuffer A serialized PersistedEntityData struct.
     * @param dataSize The size of entityDataBuffer.
     * Note: We would normally use a std::span, but it would be extra work 
     *       in this case for no gain.
     */
    void saveEntityData(entt::entity entity, Uint8* entityDataBuffer,
                        std::size_t dataSize);

    /**
     * Attempts to delete an entity table entry for the given entity.
     *
     * If the entity is not found in the database, does nothing.
     */
    void deleteEntityData(entt::entity entity);

    /**
     * Calls the given callback on each entity data entry.
     *
     * @param callback A callback of form void(entt::entity, const Uint8*,
     *                 std::size_t) that expects the entity's ID, and a 
     *                 serialized PersistedEntityData struct.
     */
    template<typename Func>
    void iterateEntities(Func callback)
    {
        while (iterateEntitiesQuery->executeStep()) {
            SQLite::Column dataColumn{iterateEntitiesQuery->getColumn(1)};
            callback(static_cast<entt::entity>(
                         iterateEntitiesQuery->getColumn(0).getInt()),
                     static_cast<const Uint8*>(dataColumn.getBlob()),
                     dataColumn.getBytes());
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
     * @param itemDataBuffer A serialized Item struct.
     * @param dataSize The size of itemDataBuffer.
     */
    void saveItemData(ItemID itemID, Uint8* itemDataBuffer,
                      std::size_t dataSize);

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
     * @param entityStoredValueIDMapBuffer A serialized 
     *                                     World::entityStoredValueIDMap.
     * @param dataSize The size of entityStoredValueIDDataBuffer.
     */
    void saveEntityStoredValueIDMap(Uint8* entityStoredValueIDMapBuffer,
                                    std::size_t dataSize);

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
     * @param globalStoredValueMapBuffer A serialized 
     *                                   World::globalStoredValueMap.
     * @param dataSize The size of globalStoredValueMapBuffer.
     */
    void saveGlobalStoredValueMap(Uint8* globalStoredValueMapBuffer,
                                  std::size_t dataSize);

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
     * Creates our tables, if they don't already exist.
     *
     * Note: If things ever get sufficiently complicated, we can switch to a 
     *       schema.
     */
    void initTables();

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

    /** File database. Used to persist our data to a file. */
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
