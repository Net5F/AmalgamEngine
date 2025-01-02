#pragma once

#include "MigrationRunner.h"

namespace SQLite
{
class Database;
}

namespace AM
{
namespace DM
{

class MigrateEngineComponents
{
public:
    /**
     * Migrates any engine entity components in the given database from 
     * currentVersion to desiredVersion.
     */
    static MigrationStatus migrate(SQLite::Database& database,
                                   unsigned int currentVersion,
                                   unsigned int desiredVersion);

private:
    static void migrateV0ToV1(SQLite::Database& database);
};

} // End namespace DM
} // End namespace AM
