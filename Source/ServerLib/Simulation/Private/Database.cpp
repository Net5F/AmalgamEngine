#include "Database.h"
#include "EnginePersistedComponentTypes.h"
#include "ProjectPersistedComponentTypes.h"
#include "Paths.h"
#include "AMAssert.h"
#include "Log.h"
#include "SQLiteCpp/VariadicBind.h"
#include "SQLiteCpp/Backup.h"

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
    // If any of our tables don't exist in Database.db3, initialize them.
    initTables();

    // Load the data from Database.db3 into our in-memory database.
    SQLite::Backup backup(database, backupDatabase);
    backup.executeStep(-1);

    // Note: We build these queries after initTables() because they'll 
    //       segfault if there's no DB with the expected fields.
    insertEntityQuery = std::make_unique<SQLite::Statement>(
        database, "INSERT INTO entities VALUES (?, ?, ?) "
                  "ON CONFLICT(id) DO UPDATE SET "
                  "engineComponents=excluded.engineComponents, "
                  "projectComponents=excluded.projectComponents");
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

void Database::saveEntityData(entt::entity entity,
                              std::span<const Uint8> engineComponentData,
                              std::span<const Uint8> projectComponentData)
{
    try {
        insertEntityQuery->bind(1, static_cast<Uint32>(entity));
        insertEntityQuery->bind(2, engineComponentData.data(),
                                static_cast<int>(engineComponentData.size()));
        insertEntityQuery->bind(3, projectComponentData.data(),
                                static_cast<int>(projectComponentData.size()));

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

void Database::saveItemData(ItemID itemID, std::span<const Uint8> itemData)
{
    try {
        insertItemQuery->bind(1, static_cast<Uint32>(itemID));
        insertItemQuery->bind(2, itemData.data(),
                              static_cast<int>(itemData.size()));

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

void Database::saveEntityStoredValueIDMap(
    std::span<const Uint8> entityStoredValueIDMapData)
{
    try {
        insertEntityStoredValueIDMapQuery->bind(
            1, entityStoredValueIDMapData.data(),
            static_cast<int>(entityStoredValueIDMapData.size()));

        insertEntityStoredValueIDMapQuery->exec();

        insertEntityStoredValueIDMapQuery->reset();
    } catch (std::exception& e) {
        LOG_ERROR("Failed to save entity stored value ID map data: %s",
                  e.what());
    }
}

void Database::saveGlobalStoredValueMap(
    std::span<const Uint8> globalStoredValueMapData)
{
    try {
        insertGlobalStoredValueMapQuery->bind(
            1, globalStoredValueMapData.data(),
            static_cast<int>(globalStoredValueMapData.size()));

        insertGlobalStoredValueMapQuery->exec();

        insertGlobalStoredValueMapQuery->reset();
    } catch (std::exception& e) {
        LOG_ERROR("Failed to save global stored value map data: %s", e.what());
    }
}

void Database::initTables()
{
    // The below commands define the schema for Database.db3.
    // Note: We only need to init the file-backed database, since the in-memory
    //       database will copy it.
    try {
        // Version numbers for all data in this database that needs to support 
        // migration.
        if (!backupDatabase.tableExists("versions")) {
            backupDatabase.exec(
                "CREATE TABLE versions (id INTEGER PRIMARY KEY, "
                "name TEXT, versionNumber INTEGER)");

            SQLite::Statement insertVersionQuery{
                backupDatabase, "INSERT INTO versions VALUES (?, ?, ?)"};

            int versionsKey{0};
            insertVersionQuery.bind(1, versionsKey++);
            insertVersionQuery.bind(2, "EngineComponents");
            insertVersionQuery.bind(3, ENGINE_COMPONENTS_VERSION);
            insertVersionQuery.exec();
            insertVersionQuery.reset();

            insertVersionQuery.bind(1, versionsKey++);
            insertVersionQuery.bind(2, "ProjectComponents");
            insertVersionQuery.bind(3, PROJECT_COMPONENTS_VERSION);
            insertVersionQuery.exec();
            insertVersionQuery.reset();
        }

        // Entity components.
        if (!backupDatabase.tableExists("entities")) {
            // The component lists need to be serialized separately, so we can 
            // migrate them separately. If we tried to serialize them together, 
            // there may be situations where both lists need to be updated at 
            // the same time, which we can't do with a split migration setup.
            backupDatabase.exec(
                "CREATE TABLE entities (id INTEGER PRIMARY KEY, "
                "engineComponents BLOB, projectComponents BLOB)");
        }

        // Items.
        if (!backupDatabase.tableExists("items")) {
            backupDatabase.exec(
                "CREATE TABLE items (id INTEGER PRIMARY KEY, data BLOB)");
        }

        // Entity stored values, stored as a single serialized map.
        if (!backupDatabase.tableExists("entityStoredValueIDMap")) {
            backupDatabase.exec(
                "CREATE TABLE entityStoredValueIDMap (data BLOB)");
            // Since we're only storing 1 value in this table, we init the row
            // here so we can use UPDATEs later.
            backupDatabase.exec(
                "INSERT INTO entityStoredValueIDMap VALUES('')");
        }

        // Global stored values, stored as a single serialized map.
        if (!backupDatabase.tableExists("globalStoredValueMap")) {
            backupDatabase.exec(
                "CREATE TABLE globalStoredValueMap (data BLOB)");
            // Since we're only storing 1 value in this table, we init the row
            // here so we can use UPDATEs later.
            backupDatabase.exec("INSERT INTO globalStoredValueMap VALUES('')");
        }
    } catch (std::exception& e) {
        LOG_ERROR("Failed to init table: %s", e.what());
    }
}

void Database::checkDataVersions()
{
    // Iterate through each version row.
    SQLite::Statement getVersionQuery{database, "SELECT * FROM versions"};
    std::vector<std::string> requiredMigrations{};
    while (getVersionQuery.executeStep()) {
        const char* name{getVersionQuery.getColumn(1).getText()};
        unsigned int versionNumber{
            static_cast<unsigned int>(getVersionQuery.getColumn(2).getInt())};

        // Match the name to one of our expected names and check the version 
        // number. If it's newer than the code, fail immediately. If it's 
        // older, push the required migrations.
        if (std::strcmp(name, "EngineComponents") == 0) {
            if (versionNumber > ENGINE_COMPONENTS_VERSION) {
                LOG_FATAL("Database load error: Data version (v%u) is newer "
                          "than code (v%u) (EngineComponents).",
                          versionNumber, ENGINE_COMPONENTS_VERSION);
            }
            else if (versionNumber < ENGINE_COMPONENTS_VERSION) {
                requiredMigrations.push_back(
                    "EngineComponents v" + std::to_string(versionNumber)
                    + " -> v" + std::to_string(ENGINE_COMPONENTS_VERSION));
            }
        }
        else if (std::strcmp(name, "ProjectComponents") == 0) {
            std::string dataVersion{std::to_string(versionNumber)};
            std::string codeVersion{std::to_string(PROJECT_COMPONENTS_VERSION)};
            if (versionNumber > PROJECT_COMPONENTS_VERSION) {
                LOG_FATAL("Database load error: Data version (v%u) is newer "
                          "than code (v%u) (ProjectComponents).",
                          versionNumber, PROJECT_COMPONENTS_VERSION);
            }
            else if (versionNumber < PROJECT_COMPONENTS_VERSION) {
                requiredMigrations.push_back(
                    "ProjectComponents v" + std::to_string(versionNumber)
                    + " -> v" + std::to_string(PROJECT_COMPONENTS_VERSION));
            }
        }
    }

    // If any data is out of date, print the required migrations and exit.
    if (!(requiredMigrations.empty())) {
        std::string errorText{"Database load error: Data version is older than "
                              "code version.\nRequired migrations:"};
        for (const std::string migrationText : requiredMigrations) {
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
