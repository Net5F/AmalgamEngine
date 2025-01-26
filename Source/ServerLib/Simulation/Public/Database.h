#pragma once

#include "ItemID.h"
#include "IconID.h"
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
#include <string_view>

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
     * @param serializedEngineComponents A serialized
     * std::vector<EnginePersistedComponent>.
     * @param serializedrojectComponents A serialized 
     * std::vector<ProjectPersistedComponent>.
     */
    void saveEntityData(entt::entity entity,
                        std::span<const Uint8> serializedEngineComponents,
                        std::span<const Uint8> serializedProjectComponents);

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
            SQLite::Column idColumn{iterateEntitiesQuery->getColumn(0)};
            SQLite::Column engineComponentsColumn{
                iterateEntitiesQuery->getColumn(1)};
            SQLite::Column projectComponentsColumn{
                iterateEntitiesQuery->getColumn(2)};
            callback(
                static_cast<entt::entity>(idColumn.getInt()),
                std::span<const Uint8>{
                    static_cast<const Uint8*>(engineComponentsColumn.getBlob()),
                    static_cast<std::size_t>(
                        engineComponentsColumn.getBytes())},
                std::span<const Uint8>{
                    static_cast<const Uint8*>(
                        projectComponentsColumn.getBlob()),
                    static_cast<std::size_t>(
                        projectComponentsColumn.getBytes())});
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
     */
    void saveItemData(ItemID itemID, std::span<const Uint8> serializedItem,
                      ItemVersion version, std::string_view initScript);

    /**
     * Attempts to delete an item table entry for the given item.
     *
     * If the item is not found in the database, does nothing.
     */
    void deleteItemData(ItemID itemID);

    /**
     * Calls the given callback on each item data entry.
     *
     * @param callback A callback of form void(ItemID, std::span<const Uint8>, 
     *                 ItemVersion, std::string_view) that expects the item's ID,
     *                 a serialized Item struct, and the item's version and init 
     *                 script.
     */
    template<typename Func>
    void iterateItems(Func callback)
    {
        while (iterateItemsQuery->executeStep()) {
            SQLite::Column idColumn{iterateItemsQuery->getColumn(0)};
            SQLite::Column itemColumn{iterateItemsQuery->getColumn(1)};
            SQLite::Column versionColumn{iterateItemsQuery->getColumn(2)};
            SQLite::Column initScriptColumn{iterateItemsQuery->getColumn(3)};
            callback(static_cast<ItemID>(idColumn.getInt()),
                     std::span<const Uint8>{
                         static_cast<const Uint8*>(itemColumn.getBlob()),
                         static_cast<std::size_t>(itemColumn.getBytes())},
                     static_cast<ItemVersion>(versionColumn.getInt()),
                     std::string_view{initScriptColumn.getText(),
                                      static_cast<std::size_t>(
                                          initScriptColumn.getBytes())});
        }
        iterateItemsQuery->reset();
    }

    //-------------------------------------------------------------------------
    // Stored Values
    //-------------------------------------------------------------------------
    /**
     * Adds or overwrites the entity stored value ID map entry.
     *
     * @param serializedMap A serialized World::entityStoredValueIDMap.
     */
    void saveEntityStoredValueIDMap(std::span<const Uint8> serializedMap);

    /**
     * Calls the given callback on the entity stored value ID map data entry.
     *
     * @param callback A callback of form void(std::span<const Uint8>) that 
     *                 expects a serialized EntityStoredValueIDMap.
     */
    template<typename Func>
    void getEntityStoredValueIDMap(Func callback)
    {
        getEntityStoredValueIDMapQuery->executeStep();
        SQLite::Column mapColumn{getEntityStoredValueIDMapQuery->getColumn(0)};
        callback(std::span<const Uint8>{
            static_cast<const Uint8*>(mapColumn.getBlob()),
            static_cast<std::size_t>(mapColumn.getBytes())});

        getEntityStoredValueIDMapQuery->reset();
    }

    /**
     * Adds or overwrites the global stored value map entry.
     *
     * @param serializedMap A serialized World::globalStoredValueMap.
     */
    void saveGlobalStoredValueMap(std::span<const Uint8> serializedMap);

    /**
     * Calls the given callback on the global stored value map data entry.
     *
     * @param callback A callback of form void(std::span<const Uint8>) that 
     *                 expects a serialized GlobalStoredValueMap.
     */
    template<typename Func>
    void getGlobalStoredValueMap(Func callback)
    {
        getGlobalStoredValueMapQuery->executeStep();
        SQLite::Column mapColumn{getGlobalStoredValueMapQuery->getColumn(0)};
        callback(std::span<const Uint8>{
            static_cast<const Uint8*>(mapColumn.getBlob()),
            static_cast<std::size_t>(mapColumn.getBytes())});

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
