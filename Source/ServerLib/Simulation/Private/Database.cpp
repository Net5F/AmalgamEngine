#include "Database.h"
#include "SQLiteCpp/VariadicBind.h"
#include "SQLiteCpp/Backup.h"
#include "Paths.h"
#include "AMAssert.h"
#include "Log.h"

#include <sqlite3.h>

#ifdef SQLITECPP_ENABLE_ASSERT_HANDLER
namespace SQLite
{
void assertion_failed(char const* apFile, int apLine, char const* apFunc,
                      char const* apExpr, char const* apMsg)
{
    LOG_ERROR("%s: %s: error: assertion failed(%s) in %s() with message \"%s\"",
              apFile, apLine, apExpr, apFunc, apMsg);
}
} // namespace SQLite
#endif

namespace AM
{
namespace Server
{
Database::Database()
: database{":memory:", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE}
, backupDatabase{(Paths::BASE_PATH + "/Database.db3"),
                 SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE}
, currentTransaction{}
, backupThreadObj{}
, exitRequested{false} 
, backupMutex{}
, backupCondVar{}
, backupRequested{false}
, insertEntityQuery{nullptr}
, deleteEntityQuery{nullptr}
, iterateEntitiesQuery{nullptr}
, insertItemQuery{nullptr}
, deleteItemQuery{nullptr}
, iterateItemsQuery{nullptr}
, insertEntityStoredValueIDMapQuery{nullptr}
, getEntityStoredValueIDMapQuery{nullptr}
, insertGlobalStoredValueMapQuery{nullptr}
, getGlobalStoredValueMapQuery{nullptr}
{
    initTables();

    // Note: We build these queries after initTables() because they'll 
    //       segfault if there's no DB with the expected fields.
    insertEntityQuery = std::make_unique<SQLite::Statement>(
        database, "INSERT INTO entities VALUES (?, ?) "
                  "ON CONFLICT(id) DO UPDATE SET data=excluded.data");
    deleteEntityQuery = std::make_unique<SQLite::Statement>(
        database, "DELETE FROM entities WHERE id=?");
    iterateEntitiesQuery = std::make_unique<SQLite::Statement>(
        backupDatabase, "SELECT * FROM entities");

    insertItemQuery = std::make_unique<SQLite::Statement>(
        database, "INSERT INTO items VALUES (?, ?) "
                  "ON CONFLICT(id) DO UPDATE SET data=excluded.data");
    deleteItemQuery = std::make_unique<SQLite::Statement>(
        database, "DELETE FROM items WHERE id=?");
    iterateItemsQuery = std::make_unique<SQLite::Statement>(
        backupDatabase, "SELECT * FROM items");

    insertEntityStoredValueIDMapQuery = std::make_unique<SQLite::Statement>(
        database, "UPDATE entityStoredValueIDMap SET data=(?)");
    getEntityStoredValueIDMapQuery = std::make_unique<SQLite::Statement>(
        backupDatabase, "SELECT * FROM entityStoredValueIDMap");

    insertGlobalStoredValueMapQuery = std::make_unique<SQLite::Statement>(
        database, "UPDATE globalStoredValueMap SET data=(?)");
    getGlobalStoredValueMapQuery = std::make_unique<SQLite::Statement>(
        backupDatabase, "SELECT * FROM globalStoredValueMap");

    // Start the backup thread.
    backupThreadObj = std::thread(&Database::performBackup, this);
}

Database::~Database()
{
    exitRequested = true;

    {
        std::unique_lock lock{backupMutex};
        backupRequested = true;
    }
    backupCondVar.notify_one();
    backupThreadObj.join();
}

void Database::startTransaction()
{
    if (currentTransaction) {
        LOG_ERROR("Tried to start a transaction while one was ongoing.");
        return;
    }

    currentTransaction.emplace(database);
}

void Database::commitTransaction()
{
    if (!currentTransaction) {
        LOG_ERROR("Tried to commit a transaction when no transaction was "
                  "ongoing.");
    }

    try {
        currentTransaction.value().commit();
    } catch (std::exception& e) {
        LOG_ERROR("Failed to commit transaction: %s", e.what());
    }

    currentTransaction.reset();
}

void Database::backupToFile()
{
    if (backupRequested) {
        LOG_INFO("Tried to begin database backup while a backup was already "
                 "underway.");
        return;
    }

    // Wake the backup thread.
    {
        std::unique_lock lock{backupMutex};
        backupRequested = true;
    }
    backupCondVar.notify_one();
}

bool Database::backupIsInProgress()
{
    return backupRequested.load();
}

void Database::saveEntityData(entt::entity entity, Uint8* entityDataBuffer,
                              std::size_t dataSize)
{
    try {
        insertEntityQuery->bind(1, static_cast<Uint32>(entity));
        insertEntityQuery->bind(2, entityDataBuffer,
                                static_cast<int>(dataSize));

        insertEntityQuery->exec();

        insertEntityQuery->reset();
    } catch (std::exception& e) {
        LOG_ERROR("Failed to save entity data: %s", e.what());
    }
}

void Database::deleteEntityData(entt::entity entity)
{
    try {
        deleteEntityQuery->bind(1, static_cast<Uint32>(entity));

        deleteEntityQuery->exec();

        deleteEntityQuery->reset();
    } catch (std::exception& e) {
        LOG_ERROR("Failed to delete entity data: %s", e.what());
    }
}

void Database::saveItemData(ItemID itemID, Uint8* entityDataBuffer,
                              std::size_t dataSize)
{
    try {
        insertItemQuery->bind(1, static_cast<Uint32>(itemID));
        insertItemQuery->bind(2, entityDataBuffer, static_cast<int>(dataSize));

        insertItemQuery->exec();

        insertItemQuery->reset();
    } catch (std::exception& e) {
        LOG_ERROR("Failed to save item data: %s", e.what());
    }
}

void Database::deleteItemData(ItemID itemID)
{
    try {
        deleteItemQuery->bind(1, static_cast<Uint32>(itemID));

        deleteItemQuery->exec();

        deleteItemQuery->reset();
    } catch (std::exception& e) {
        LOG_ERROR("Failed to delete item data: %s", e.what());
    }
}

void Database::saveEntityStoredValueIDMap(Uint8* entityStoredValueIDMapBuffer,
                                          std::size_t dataSize)
{
    try {
        insertEntityStoredValueIDMapQuery->bind(1, entityStoredValueIDMapBuffer,
                                                static_cast<int>(dataSize));

        insertEntityStoredValueIDMapQuery->exec();

        insertEntityStoredValueIDMapQuery->reset();
    } catch (std::exception& e) {
        LOG_ERROR("Failed to save entity stored value ID map data: %s",
                  e.what());
    }
}

void Database::saveGlobalStoredValueMap(Uint8* globalStoredValueMapBuffer,
                                        std::size_t dataSize)
{
    try {
        insertGlobalStoredValueMapQuery->bind(1, globalStoredValueMapBuffer,
                                              static_cast<int>(dataSize));

        insertGlobalStoredValueMapQuery->exec();

        insertGlobalStoredValueMapQuery->reset();
    } catch (std::exception& e) {
        LOG_ERROR("Failed to save global stored value map data: %s", e.what());
    }
}

void Database::initTables()
{
    try {
        if (!(database.tableExists("entities"))) {
            database.exec(
                "CREATE TABLE entities (id INTEGER PRIMARY KEY, data BLOB)");
        }

        if (!(database.tableExists("items"))) {
            database.exec(
                "CREATE TABLE items (id INTEGER PRIMARY KEY, data BLOB)");
        }

        if (!(database.tableExists("entityStoredValueIDMap"))) {
            database.exec("CREATE TABLE entityStoredValueIDMap (data BLOB)");
            database.exec("INSERT INTO entityStoredValueIDMap VALUES('')");
        }

        if (!(database.tableExists("globalStoredValueMap"))) {
            database.exec("CREATE TABLE globalStoredValueMap (data BLOB)");
            database.exec("INSERT INTO globalStoredValueMap VALUES('')");
        }

        // Note: We need to init the backup database for the first load, before
        //       any backups have been performed.
        if (!(backupDatabase.tableExists("entities"))) {
            backupDatabase.exec(
                "CREATE TABLE entities (id INTEGER PRIMARY KEY, data BLOB)");
        }

        if (!(backupDatabase.tableExists("items"))) {
            backupDatabase.exec(
                "CREATE TABLE items (id INTEGER PRIMARY KEY, data BLOB)");
        }

        if (!(backupDatabase.tableExists("entityStoredValueIDMap"))) {
            backupDatabase.exec(
                "CREATE TABLE entityStoredValueIDMap (data BLOB)");
            backupDatabase.exec(
                "INSERT INTO entityStoredValueIDMap VALUES('')");
        }

        if (!(backupDatabase.tableExists("globalStoredValueMap"))) {
            backupDatabase.exec(
                "CREATE TABLE globalStoredValueMap (data BLOB)");
            backupDatabase.exec(
                "INSERT INTO globalStoredValueMap VALUES('')");
        }
    } catch (std::exception& e) {
        LOG_ERROR("Failed to init table: %s", e.what());
    }
}

void Database::performBackup()
{
    while (!exitRequested) {
        // Wait until this thread is signaled by backupToFile().
        std::unique_lock lock{backupMutex};
        backupCondVar.wait(lock, [this] { return backupRequested.load(); });

        // Execute all backup steps at once.
        try {
            SQLite::Backup backup(backupDatabase, database);
            backup.executeStep();
        } catch (std::exception& e) {
            LOG_ERROR("Failed to save database to file: %s", e.what());
        }

        backupRequested = false;
    }
}

} // namespace Server
} // namespace AM
