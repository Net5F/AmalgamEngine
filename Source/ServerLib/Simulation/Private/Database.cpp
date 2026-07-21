#include "Database.h"
#include "PersistedComponentDefs.h"
#include "Paths.h"
#include "AMAssert.h"
#include "Log.h"
#include "SQLiteCpp/VariadicBind.h"
#include "SQLiteCpp/Backup.h"
#include <sqlite3.h>
#include <array>

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
, backupDatabase{(Paths::BASE_PATH + "/World.db"),
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
    // If any of our tables don't exist in World.db, initialize them.
    initTables();

    // Load the data from World.db into our in-memory database.
    SQLite::Backup backup(database, backupDatabase);
    backup.executeStep(-1);

    // Note: We build these queries after initTables() because they'll
    //       segfault if there's no DB with the expected fields.
    insertEntityQuery = std::make_unique<SQLite::Statement>(database, R"(
            INSERT INTO entities VALUES (?, ?, ?)
            ON CONFLICT(id) DO UPDATE SET
                serialized_engine_components=
                    excluded.serialized_engine_components,
                serialized_project_components=
                    excluded.serialized_project_components
        )");
    deleteEntityQuery = std::make_unique<SQLite::Statement>(
        database, "DELETE FROM entities WHERE id=?");
    iterateEntitiesQuery = std::make_unique<SQLite::Statement>(
        backupDatabase, "SELECT * FROM entities");

    insertItemQuery = std::make_unique<SQLite::Statement>(
        database, R"(
            INSERT INTO items VALUES (?, ?, ?, ?)
            ON CONFLICT(id) DO UPDATE SET
                serialized_item=excluded.serialized_item,
                version=excluded.version,
                init_script=excluded.init_script
        )");
    deleteItemQuery = std::make_unique<SQLite::Statement>(
        database, "DELETE FROM items WHERE id=?");
    iterateItemsQuery = std::make_unique<SQLite::Statement>(
        backupDatabase, "SELECT * FROM items");

    insertEntityStoredValueIDMapQuery = std::make_unique<SQLite::Statement>(
        database, "UPDATE entity_stored_value_id_map SET serialized_map=(?)");
    getEntityStoredValueIDMapQuery = std::make_unique<SQLite::Statement>(
        backupDatabase, "SELECT * FROM entity_stored_value_id_map");

    insertGlobalStoredValueMapQuery = std::make_unique<SQLite::Statement>(
        database, "UPDATE global_stored_value_map SET serialized_map=(?)");
    getGlobalStoredValueMapQuery = std::make_unique<SQLite::Statement>(
        backupDatabase, "SELECT * FROM global_stored_value_map");

    // Check for any out of date data.
    checkDataVersions();

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

void Database::saveEntityData(
    entt::entity entity, std::span<const Uint8> serializedEngineComponents,
    std::span<const Uint8> serializedProjectComponents)
{
    try {
        insertEntityQuery->bind(1, static_cast<int>(entity));
        insertEntityQuery->bind(
            2, serializedEngineComponents.data(),
            static_cast<int>(serializedEngineComponents.size()));
        insertEntityQuery->bind(
            3, serializedProjectComponents.data(),
            static_cast<int>(serializedProjectComponents.size()));

        insertEntityQuery->exec();

        insertEntityQuery->reset();
    } catch (std::exception& e) {
        LOG_ERROR("Failed to save entity data: %s", e.what());
    }
}

void Database::deleteEntityData(entt::entity entity)
{
    try {
        deleteEntityQuery->bind(1, static_cast<int>(entity));

        deleteEntityQuery->exec();

        deleteEntityQuery->reset();
    } catch (std::exception& e) {
        LOG_ERROR("Failed to delete entity data: %s", e.what());
    }
}

void Database::saveItemData(ItemID itemID,
                            std::span<const Uint8> serializedItem,
                            ItemVersion version, std::string_view initScript)
{
    try {
        insertItemQuery->bind(1, static_cast<int>(itemID));
        insertItemQuery->bind(2, serializedItem.data(),
                              static_cast<int>(serializedItem.size()));
        insertItemQuery->bind(3, static_cast<int>(version));
        insertItemQuery->bind(4, initScript.data(),
                              static_cast<int>(initScript.size()));

        insertItemQuery->exec();

        insertItemQuery->reset();
    } catch (std::exception& e) {
        LOG_ERROR("Failed to save item data: %s", e.what());
    }
}

void Database::deleteItemData(ItemID itemID)
{
    try {
        deleteItemQuery->bind(1, static_cast<int>(itemID));

        deleteItemQuery->exec();

        deleteItemQuery->reset();
    } catch (std::exception& e) {
        LOG_ERROR("Failed to delete item data: %s", e.what());
    }
}

void Database::saveEntityStoredValueIDMap(std::span<const Uint8> serializedMap)
{
    try {
        insertEntityStoredValueIDMapQuery->bind(
            1, serializedMap.data(), static_cast<int>(serializedMap.size()));

        insertEntityStoredValueIDMapQuery->exec();

        insertEntityStoredValueIDMapQuery->reset();
    } catch (std::exception& e) {
        LOG_ERROR("Failed to save entity stored value ID map data: %s",
                  e.what());
    }
}

void Database::saveGlobalStoredValueMap(std::span<const Uint8> serializedMap)
{
    try {
        insertGlobalStoredValueMapQuery->bind(
            1, serializedMap.data(), static_cast<int>(serializedMap.size()));

        insertGlobalStoredValueMapQuery->exec();

        insertGlobalStoredValueMapQuery->reset();
    } catch (std::exception& e) {
        LOG_ERROR("Failed to save global stored value map data: %s", e.what());
    }
}

void Database::initTables()
{
    // The below commands define the schema for World.db.
    // Note: We only need to init the file-backed database, since the in-memory
    //       database will copy it.
    try {
        // Version numbers for all data in this database that needs to support
        // migration.
        if (!backupDatabase.tableExists("versions")) {
            backupDatabase.exec(R"(
                CREATE TABLE versions
                (
                    id              INTEGER PRIMARY KEY,
                    name            TEXT,
                    version_number  INTEGER
                ) STRICT
            )");

            SQLite::Statement insertVersionQuery{
                backupDatabase, "INSERT INTO versions VALUES (?, ?, ?)"};

            int versionsKey{0};
            insertVersionQuery.bind(1, versionsKey++);
            insertVersionQuery.bind(2, "PersistedComponents");
            insertVersionQuery.bind(3, PERSISTED_COMPONENTS_VERSION);
            insertVersionQuery.exec();
            insertVersionQuery.reset();
        }

        // Entity components.
        if (!backupDatabase.tableExists("entities")) {
            // Engine and project components use separate ID namespaces, so
            // store their record lists in separate blobs.
            backupDatabase.exec(R"(
                CREATE TABLE entities
                (
                    id                             INTEGER PRIMARY KEY,
                    serialized_engine_components  BLOB,
                    serialized_project_components BLOB
                ) STRICT
            )");
        }

        // Items.
        if (!backupDatabase.tableExists("items")) {
            backupDatabase.exec(R"(
                CREATE TABLE items
                (
                    id               INTEGER PRIMARY KEY,
                    serialized_item  BLOB,
                    version          INTEGER,
                    init_script      TEXT
                ) STRICT
            )");
        }

        // Entity stored values, stored as a single serialized map.
        if (!backupDatabase.tableExists("entity_stored_value_id_map")) {
            backupDatabase.exec(R"(
                CREATE TABLE entity_stored_value_id_map
                (
                    serialized_map BLOB
                ) STRICT
            )");
            // Since we're only storing 1 value in this table, we init the row
            // here so we can use UPDATEs later.
            backupDatabase.exec(
                "INSERT INTO entity_stored_value_id_map VALUES('')");
        }

        // Global stored values, stored as a single serialized map.
        if (!backupDatabase.tableExists("global_stored_value_map")) {
            backupDatabase.exec(R"(
                CREATE TABLE global_stored_value_map
                (
                    serialized_map BLOB
                ) STRICT
            )");
            // Since we're only storing 1 value in this table, we init the row
            // here so we can use UPDATEs later.
            backupDatabase.exec(
                "INSERT INTO global_stored_value_map VALUES('')");
        }
    } catch (std::exception& e) {
        LOG_ERROR("Failed to init table: %s", e.what());
    }
}

void Database::checkDataVersions()
{
    struct VersionInfo {
        std::string name{};
        unsigned int targetVersion{};
        bool wasFound{false};
    };
    std::array<VersionInfo, 1> targetVersions{
        {"PersistedComponents", PERSISTED_COMPONENTS_VERSION}};

    // Iterate through each version row in the database.
    SQLite::Statement getVersionQuery{database, "SELECT * FROM versions"};
    std::vector<std::string> requiredMigrations{};
    while (getVersionQuery.executeStep()) {
        const char* name{getVersionQuery.getColumn(1).getText()};
        unsigned int versionNumber{
            static_cast<unsigned int>(getVersionQuery.getColumn(2).getInt())};

        // Match this row's version name to one of our expected names and check
        // the version number. If it's newer than the code, fail immediately.
        // If it's older, add the required migration to the error string.
        for (VersionInfo& versionInfo : targetVersions) {
            if (name == versionInfo.name) {
                versionInfo.wasFound = true;

                if (versionNumber > versionInfo.targetVersion) {
                    LOG_FATAL(
                        "Database load error: Data version (v%u) is newer "
                        "than code (v%u) (%s).",
                        versionNumber, versionInfo.targetVersion,
                        versionInfo.name.c_str());
                }
                else if (versionNumber < versionInfo.targetVersion) {
                    requiredMigrations.push_back(
                        versionInfo.name + " v" + std::to_string(versionNumber)
                        + " -> v" + std::to_string(versionInfo.targetVersion));
                }
            }
        }
    }

    for (const VersionInfo& versionInfo : targetVersions) {
        if (!(versionInfo.wasFound)) {
            LOG_FATAL("Database load error: Missing %s version field.",
                      versionInfo.name.c_str());
        }
    }

    // If any data is out of date, print the required migrations and exit.
    if (!(requiredMigrations.empty())) {
        std::string errorText{"Database load error: Data version is older than "
                              "code version.\nRequired migrations:"};
        for (const std::string& migrationText : requiredMigrations) {
            errorText += "\n    " + migrationText;
        }

        LOG_FATAL("%s", errorText.c_str());
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
            backup.executeStep(-1);
        } catch (std::exception& e) {
            LOG_ERROR("Failed to save database to file: %s", e.what());
        }

        backupRequested = false;
    }
}

} // namespace Server
} // namespace AM
