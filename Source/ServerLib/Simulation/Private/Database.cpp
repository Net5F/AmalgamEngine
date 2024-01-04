#include "Database.h"
#include "Paths.h"

namespace AM
{
namespace Server
{
Database::Database()
: database{(Paths::BASE_PATH + "/Database.db3"),
           SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE}
{
    initTables();
}

void Database::initTables()
{
    if (!(database.tableExists("entities"))) {
        database.exec(
            "CREATE TABLE entities (id INTEGER PRIMARY KEY, data BLOB");
    }

    if (!(database.tableExists("items"))) {
        database.exec(
            "CREATE TABLE items (id INTEGER PRIMARY KEY, data BLOB");
    }
}

} // namespace Server
} // namespace AM
