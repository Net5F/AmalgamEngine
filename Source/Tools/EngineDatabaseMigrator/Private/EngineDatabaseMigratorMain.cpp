#include "MigrationRunner.h"
#include "MigrateEngineComponents.h"
#include "EnginePersistedComponentTypes.h"
#include "Log.h"

#include "SQLiteCpp/SQLiteCpp.h"
#include <SDL_stdinc.h>
#include <iostream>

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

using namespace AM;
// Note: Database migrations are only relevant to the server, so we shouldn't 
//       hit any clashes with the Client namespace.
using namespace AM::Server;
using namespace AM::DM;

/**
 * Migration info for each piece of database data that the engine controls.
 */
std::vector<MigrationInfo> engineMigrationInfo{
    {"EngineComponents", ENGINE_COMPONENTS_VERSION,
     MigrateEngineComponents::migrate}};

int main(int argc, char** argv)
{
    if (argc != 2) {
        std::printf("Too few arguments.\n");
        std::printf("Usage: EngineDatabaseMigrator.exe <DatabasePath>\n"
                    "  Updates all engine-controlled data in the given "
                    "Database.db3 file to the latest version.\n");
        std::fflush(stdout);
        return 1;
    }

    // Open the given database file.
    std::unique_ptr<SQLite::Database> database{};
    try {
        database = std::make_unique<SQLite::Database>(argv[1],
                                                      SQLite::OPEN_READWRITE);
    } catch (std::exception&) {
        std::printf("Invalid database file: %s.\nMust be Amalgam Engine's "
                    "Database.db3\n",
                    argv[1]);
        return 1;
    }

    // Perform any necessary migrations.
    MigrationRunner::migrate(*database, engineMigrationInfo);

    return 0;
}
