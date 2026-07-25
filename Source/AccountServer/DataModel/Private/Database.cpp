#include "Database.h"
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
namespace AccountServer
{
Database::Database()
: database{(Paths::BASE_PATH + "/Accounts.db"),
           SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE}
, registerAccountQuery{nullptr}
, insertRecoveryKeyQuery{nullptr}
{
    // If any of our tables don't exist in Accounts.db, initialize them.
    initTables();

    // Note: We build these queries after initTables() because they'll
    //       segfault if there's no DB with the expected fields.
    registerAccountQuery = std::make_unique<SQLite::Statement>(database, R"(
            INSERT INTO accounts
                (username, normalized_username, password_hash,
                 created_at, updated_at)
            VALUES
                (:username, lower(:username), :password_hash,
                 unixepoch(), unixepoch())
            ON CONFLICT(normalized_username) DO NOTHING
        )");

    insertRecoveryKeyQuery
        = std::make_unique<SQLite::Statement>(database, R"(
            INSERT INTO account_recovery_keys
                (account_id, key_hash, created_at)
            VALUES
                (:account_id, :key_hash, unixepoch())
        )");
}

SQLite::Transaction Database::startTransaction()
{
    return SQLite::Transaction(database);
}

SQLite::Transaction Database::startTransaction(SQLite::TransactionBehavior behavior)
{
    return SQLite::Transaction(database, behavior);
}

Database::RegisterResult
    Database::registerAccount(const std::string& username,
                              const std::string& passwordHash,
                              const std::string& recoveryKeyHash)
{
    try {
        SQLite::Transaction transaction{database};

        // Insert the username/password.
        registerAccountQuery->bind(":username", username);
        registerAccountQuery->bind(":password_hash", passwordHash);

        int changedRowCount{registerAccountQuery->exec()};

        registerAccountQuery->reset();
        if (changedRowCount == 0) {
            return RegisterResult::UsernameUnavailable;
        }

        // Insert the recovery key.
        insertRecoveryKeyQuery->bind(":account_id",
                                     database.getLastInsertRowid());
        insertRecoveryKeyQuery->bind(":key_hash", recoveryKeyHash.data(),
                                     static_cast<int>(recoveryKeyHash.size()));
        insertRecoveryKeyQuery->exec();
        insertRecoveryKeyQuery->reset();

        transaction.commit();
        return RegisterResult::Success;
    } catch (std::exception& e) {
        // Ensure the pre-built statement is reusable after a failed insert.
        registerAccountQuery->tryReset();
        insertRecoveryKeyQuery->tryReset();
        LOG_ERROR("Failed to register account: %s", e.what());
        return RegisterResult::DatabaseError;
    }
}

void Database::initTables()
{
    // The below commands define the schema for Accounts.db.
    try {
        // Info on which migrations have been applied to this database.
        if (!database.tableExists("schema_migrations")) {
            database.exec(R"(
                CREATE TABLE schema_migrations
                (
                    version INTEGER PRIMARY KEY,
                    applied_at INTEGER NOT NULL
                ) STRICT
            )");
        }

        // Account info.
        if (!database.tableExists("accounts")) {
            database.exec(R"(
                CREATE TABLE accounts
                (
                    account_id           INTEGER PRIMARY KEY AUTOINCREMENT,
                    username             TEXT NOT NULL,
                    normalized_username  TEXT NOT NULL,

                    password_hash        TEXT NOT NULL,

                    status               TEXT NOT NULL DEFAULT 'active'
                        CHECK (status IN (
                            'active',
                            'pending_email_verification',
                            'suspended',
                            'disabled'
                        )),

                    created_at           INTEGER NOT NULL,
                    updated_at           INTEGER NOT NULL,

                    CHECK (length(username) BETWEEN 3 AND 24),
                    CHECK (length(normalized_username) BETWEEN 3 AND 24)
                ) STRICT;

                CREATE UNIQUE INDEX accounts_normalized_username_uq
                    ON accounts(normalized_username);
            )");
        }

        // Account recovery keys.
        if (!database.tableExists("account_recovery_keys")) {
            database.exec(R"(
                CREATE TABLE account_recovery_keys
                (
                    recovery_key_id  INTEGER PRIMARY KEY,

                    account_id       INTEGER NOT NULL
                        REFERENCES accounts(account_id)
                        ON DELETE CASCADE,

                    key_hash         BLOB NOT NULL
                        CHECK (length(key_hash) = 32),

                    created_at       INTEGER NOT NULL,
                    used_at          INTEGER,
                    revoked_at       INTEGER,

                    CHECK (used_at IS NULL OR revoked_at IS NULL)
                ) STRICT;

                CREATE UNIQUE INDEX account_recovery_keys_one_active_uq
                    ON account_recovery_keys(account_id)
                    WHERE used_at IS NULL AND revoked_at IS NULL;
            )");
        }
    } catch (std::exception& e) {
        LOG_ERROR("Failed to init table: %s", e.what());
    }
}

} // namespace AccountServer
} // namespace AM
