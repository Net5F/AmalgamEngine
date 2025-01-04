#pragma once

#include <functional>
#include <vector>
#include <string>

namespace SQLite
{
class Database;
}

namespace AM
{
namespace DM
{

enum class MigrationStatus
{
    Success,
    /** A migration function is missing. */
    ImplementationMissing,
    /** An error occured while performing a database query. */
    DatabaseError
};

struct MigrationInfo
{
    /** The name of this piece of data. Should match the "name" column in the 
        "versions" table. */
    std::string name{};

    /** The latest version for this piece of data.
        Should be filled with a constant so it gets auto-updated. */
    unsigned int codeVersion{};

    /** The function to call to perform a migration on this piece of data. */
    std::function<MigrationStatus(SQLite::Database& database,
                                  unsigned int currentVersion,
                                  unsigned int desiredVersion)>
        migrationFunction;
};

/**
 * Runs the migration process, checking for out of date data and calling the 
 * appropriate migration functions.
 */
class MigrationRunner
{
public:
    /**
     * Performs any necessary migrations on the given database, using the given 
     * migration info.
     */
    static void migrate(SQLite::Database& database,
                        std::vector<MigrationInfo>& migrationInfoVec);
};

} // End namespace DM
} // End namespace AM
