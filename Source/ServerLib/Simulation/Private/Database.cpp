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
        LOG_ERROR(
            "Tried to commit a transaction when no transaction was ongoing.");
    }

    currentTransaction.value().commit();

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
    insertEntityQuery->bind(1, static_cast<Uint32>(entity));
    insertEntityQuery->bind(2, entityDataBuffer, static_cast<Uint32>(dataSize));

    int numExecuted{insertEntityQuery->exec()};
    AM_ASSERT(numExecuted == 1, "Failed to save entity data.");

    insertEntityQuery->reset();
}

void Database::deleteEntityData(entt::entity entity)
{
    deleteEntityQuery->bind(1, static_cast<Uint32>(entity));

    deleteEntityQuery->exec();

    deleteEntityQuery->reset();
}

void Database::saveItemData(ItemID itemID, Uint8* entityDataBuffer,
                              std::size_t dataSize)
{
    insertItemQuery->bind(1, static_cast<Uint32>(itemID));
    insertItemQuery->bind(2, entityDataBuffer, static_cast<Uint32>(dataSize));

    int numExecuted{insertItemQuery->exec()};
    AM_ASSERT(numExecuted == 1, "Failed to save item data.");

    insertItemQuery->reset();
}

void Database::deleteItemData(ItemID itemID)
{
    deleteItemQuery->bind(1, static_cast<Uint32>(itemID));

    deleteItemQuery->exec();

    deleteItemQuery->reset();
}

void Database::initTables()
{
    if (!(database.tableExists("entities"))) {
        database.exec(
            "CREATE TABLE entities (id INTEGER PRIMARY KEY, data BLOB)");
    }

    if (!(database.tableExists("items"))) {
        database.exec(
            "CREATE TABLE items (id INTEGER PRIMARY KEY, data BLOB)");
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
}

void Database::performBackup()
{
    while (!exitRequested) {
        // Wait until this thread is signaled by backupToFile().
        std::unique_lock lock{backupMutex};
        backupCondVar.wait(lock, [this] { return backupRequested.load(); });

        // Execute all backup steps at once.
        SQLite::Backup backup(backupDatabase, database);
        int result{backup.executeStep()};
        AM_ASSERT(result == SQLITE_DONE, "Failed to save database to file.");

        backupRequested = false;
    }
}

} // namespace Server
} // namespace AM
