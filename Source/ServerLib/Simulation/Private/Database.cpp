#include "Database.h"
#include "SQLiteCpp/VariadicBind.h"
#include "Paths.h"
#include "AMAssert.h"
#include "Log.h"

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
: database{(Paths::BASE_PATH + "/Database.db3"),
           SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE}
, currentTransaction{}
, insertEntityQuery{nullptr}
, deleteEntityQuery{nullptr}
, iterateEntitiesQuery{nullptr}
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
        database, "SELECT * FROM entities");
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
}

} // namespace Server
} // namespace AM
