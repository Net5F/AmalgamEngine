#include "MigrationRunner.h"
#include "SQLiteCpp/SQLiteCpp.h"
#include "Log.h"

namespace AM
{
namespace DM
{

void MigrationRunner::migrate(SQLite::Database& database,
                              std::vector<MigrationInfo>& migrationInfoVec)
{
    std::printf("Checking database for required migrations...\n");

    // Iterate through each version row.
    SQLite::Statement getVersionQuery{database, "SELECT * FROM versions"};
    while (getVersionQuery.executeStep()) {
        const char* name{getVersionQuery.getColumn(1).getText()};
        unsigned int versionNumber{
            static_cast<unsigned int>(getVersionQuery.getColumn(2).getInt())};

        // Match the name to one of our expected names and check the version 
        // number. If it's newer than the code, fail immediately. If it's 
        // older, perform the required migrations.
        for (const auto& migrationInfo : migrationInfoVec) {
            if (migrationInfo.name == name) {
                if (versionNumber > migrationInfo.codeVersion) {
                    LOG_FATAL("Migration error: %s version in database (v%u) "
                              "is newer than code (v%u).",
                              migrationInfo.name.c_str(), versionNumber,
                              migrationInfo.codeVersion);
                }
                else if (versionNumber < migrationInfo.codeVersion) {
                    MigrationStatus status{migrationInfo.migrationFunction(
                        database, versionNumber, migrationInfo.codeVersion)};
                    if (status == MigrationStatus::Success) {
                        std::printf("%s migrated to v%u\n",
                                    migrationInfo.name.c_str(),
                                    migrationInfo.codeVersion);
                    }
                    else if (status == MigrationStatus::ImplementationMissing) {
                        LOG_FATAL("Migration error: Implementation is missing "
                                  "for %s v%u -> v%u",
                                  migrationInfo.name.c_str(), versionNumber,
                                  migrationInfo.codeVersion);
                    }
                }
                else {
                    LOG_INFO("%s is up to date.", migrationInfo.name.c_str());
                }

                break;
            }
        }
    }
}

} // End namespace DM
} // End namespace AM
