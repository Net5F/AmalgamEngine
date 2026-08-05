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
, getAccountLoginInfoQuery{nullptr}
, createSessionQuery{nullptr}
, validateSessionQuery{nullptr}
, revokeSessionQuery{nullptr}
, revokeAllSessionsQuery{nullptr}
, createServiceTicketQuery{nullptr}
, consumeServiceTicketQuery{nullptr}
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

    getAccountLoginInfoQuery
        = std::make_unique<SQLite::Statement>(database, R"(
            SELECT account_id, password_hash, status
            FROM accounts
            WHERE normalized_username = lower(:username)
            LIMIT 1
        )");

    createSessionQuery = std::make_unique<SQLite::Statement>(database, R"(
            INSERT INTO account_sessions
                (account_id, token_hash, created_at, last_used_at,
                 idle_expires_at, absolute_expires_at)
            SELECT
                account_id, :token_hash, unixepoch(), unixepoch(),
                :idle_expires_at, :absolute_expires_at
            FROM accounts
            WHERE account_id = :account_id
              AND status = 'active'
        )");

    validateSessionQuery
        = std::make_unique<SQLite::Statement>(database, R"(
            UPDATE account_sessions
            SET last_used_at = unixepoch(),
                idle_expires_at = min(
                    unixepoch() + :idle_timeout_s,
                    absolute_expires_at
                )
            WHERE token_hash = :token_hash
              AND revoked_at IS NULL
              AND idle_expires_at > unixepoch()
              AND absolute_expires_at > unixepoch()
              AND EXISTS (
                  SELECT 1
                  FROM accounts
                  WHERE accounts.account_id = account_sessions.account_id
                    AND accounts.status = 'active'
              )
            RETURNING session_id, account_id, created_at, last_used_at,
                      idle_expires_at, absolute_expires_at
        )");

    revokeSessionQuery = std::make_unique<SQLite::Statement>(database, R"(
            UPDATE account_sessions
            SET revoked_at = unixepoch()
            WHERE token_hash = :token_hash
              AND revoked_at IS NULL
        )");

    revokeAllSessionsQuery
        = std::make_unique<SQLite::Statement>(database, R"(
            UPDATE account_sessions
            SET revoked_at = unixepoch()
            WHERE account_id = :account_id
              AND revoked_at IS NULL
        )");

    createServiceTicketQuery
        = std::make_unique<SQLite::Statement>(database, R"(
            INSERT INTO service_tickets
                (token_hash, account_session_id, audience, target_server_id,
                 created_at, expires_at)
            SELECT
                :token_hash, sessions.session_id, :audience,
                :target_server_id, unixepoch(), :expires_at
            FROM account_sessions AS sessions
            JOIN accounts
              ON accounts.account_id = sessions.account_id
            WHERE sessions.session_id = :account_session_id
              AND sessions.revoked_at IS NULL
              AND sessions.idle_expires_at > unixepoch()
              AND sessions.absolute_expires_at > unixepoch()
              AND accounts.status = 'active'
              AND :expires_at > unixepoch()
        )");

    consumeServiceTicketQuery
        = std::make_unique<SQLite::Statement>(database, R"(
            UPDATE service_tickets
            SET consumed_at = unixepoch()
            WHERE token_hash = :token_hash
              AND audience = :audience
              AND target_server_id = :target_server_id
              AND consumed_at IS NULL
              AND revoked_at IS NULL
              AND expires_at > unixepoch()
              AND EXISTS (
                  SELECT 1
                  FROM account_sessions AS sessions
                  JOIN accounts
                    ON accounts.account_id = sessions.account_id
                  WHERE sessions.session_id
                            = service_tickets.account_session_id
                    AND sessions.revoked_at IS NULL
                    AND sessions.idle_expires_at > unixepoch()
                    AND sessions.absolute_expires_at > unixepoch()
                    AND accounts.status = 'active'
              )
            RETURNING
                account_session_id,
                (
                    SELECT sessions.account_id
                    FROM account_sessions AS sessions
                    WHERE sessions.session_id
                              = service_tickets.account_session_id
                ),
                (
                    SELECT accounts.status
                    FROM account_sessions AS sessions
                    JOIN accounts
                      ON accounts.account_id = sessions.account_id
                    WHERE sessions.session_id
                              = service_tickets.account_session_id
                ),
                consumed_at
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

Database::AccountLoginInfo
    Database::getAccountLoginInfo(const std::string& username)
{
    AccountLoginInfo loginInfo{};

    try {
        getAccountLoginInfoQuery->bind(":username", username);

        if (!(getAccountLoginInfoQuery->executeStep())) {
            getAccountLoginInfoQuery->reset();
            loginInfo.result = AccountLoginInfo::Result::AccountNotFound;
            return loginInfo;
        }

        loginInfo.accountID
            = getAccountLoginInfoQuery->getColumn(0).getInt64();
        loginInfo.passwordHash
            = getAccountLoginInfoQuery->getColumn(1).getString();
        loginInfo.status = getAccountLoginInfoQuery->getColumn(2).getString();

        getAccountLoginInfoQuery->reset();
        loginInfo.result = AccountLoginInfo::Result::Success;
    } catch (std::exception& e) {
        getAccountLoginInfoQuery->tryReset();
        LOG_ERROR("Failed to get account login info: %s", e.what());
        loginInfo.result = AccountLoginInfo::Result::DatabaseError;
    }

    return loginInfo;
}

Database::CreateSessionResult
    Database::createSession(Sint64 accountID,
                            const std::string& tokenHash,
                            Sint64 idleExpiresAt,
                            Sint64 absoluteExpiresAt)
{
    try {
        createSessionQuery->bind(":account_id", accountID);
        createSessionQuery->bind(":token_hash", tokenHash.data(),
                                 static_cast<int>(tokenHash.size()));
        createSessionQuery->bind(":idle_expires_at", idleExpiresAt);
        createSessionQuery->bind(":absolute_expires_at", absoluteExpiresAt);

        int changedRowCount{createSessionQuery->exec()};
        createSessionQuery->reset();

        if (changedRowCount == 0) {
            return CreateSessionResult::AccountUnavailable;
        }

        return CreateSessionResult::Success;
    } catch (std::exception& e) {
        createSessionQuery->tryReset();
        LOG_ERROR("Failed to create account session: %s", e.what());
        return CreateSessionResult::DatabaseError;
    }
}

Database::AccountSessionInfo
    Database::validateSession(const std::string& tokenHash,
                              Sint64 idleTimeoutS)
{
    AccountSessionInfo sessionInfo{};

    try {
        validateSessionQuery->bind(":token_hash", tokenHash.data(),
                                   static_cast<int>(tokenHash.size()));
        validateSessionQuery->bind(":idle_timeout_s", idleTimeoutS);

        if (!(validateSessionQuery->executeStep())) {
            validateSessionQuery->reset();
            sessionInfo.result
                = AccountSessionInfo::Result::SessionNotFound;
            return sessionInfo;
        }

        sessionInfo.sessionID
            = validateSessionQuery->getColumn(0).getInt64();
        sessionInfo.accountID
            = validateSessionQuery->getColumn(1).getInt64();
        sessionInfo.createdAt
            = validateSessionQuery->getColumn(2).getInt64();
        sessionInfo.lastUsedAt
            = validateSessionQuery->getColumn(3).getInt64();
        sessionInfo.idleExpiresAt
            = validateSessionQuery->getColumn(4).getInt64();
        sessionInfo.absoluteExpiresAt
            = validateSessionQuery->getColumn(5).getInt64();

        validateSessionQuery->reset();
        sessionInfo.result = AccountSessionInfo::Result::Success;
        return sessionInfo;
    } catch (std::exception& e) {
        validateSessionQuery->tryReset();
        LOG_ERROR("Failed to validate account session: %s", e.what());
        sessionInfo.result = AccountSessionInfo::Result::DatabaseError;
        return sessionInfo;
    }
}

Database::RevokeSessionResult
    Database::revokeSession(const std::string& tokenHash)
{
    try {
        revokeSessionQuery->bind(":token_hash", tokenHash.data(),
                                 static_cast<int>(tokenHash.size()));

        int changedRowCount{revokeSessionQuery->exec()};
        revokeSessionQuery->reset();

        if (changedRowCount == 0) {
            return RevokeSessionResult::SessionNotFound;
        }

        return RevokeSessionResult::Success;
    } catch (std::exception& e) {
        revokeSessionQuery->tryReset();
        LOG_ERROR("Failed to revoke account session: %s", e.what());
        return RevokeSessionResult::DatabaseError;
    }
}

Database::RevokeAllSessionsResult
    Database::revokeAllSessions(Sint64 accountID)
{
    try {
        revokeAllSessionsQuery->bind(":account_id", accountID);
        revokeAllSessionsQuery->exec();
        revokeAllSessionsQuery->reset();
        return RevokeAllSessionsResult::Success;
    } catch (std::exception& e) {
        revokeAllSessionsQuery->tryReset();
        LOG_ERROR("Failed to revoke account sessions: %s", e.what());
        return RevokeAllSessionsResult::DatabaseError;
    }
}

Database::CreateServiceTicketResult Database::createServiceTicket(
    Sint64 accountSessionID, const std::string& tokenHash,
    ServiceTicketAudience audience, Sint64 targetServerID,
    Sint64 expiresAt)
{
    try {
        createServiceTicketQuery->bind(":account_session_id",
                                       accountSessionID);
        createServiceTicketQuery->bind(":token_hash", tokenHash.data(),
                                       static_cast<int>(tokenHash.size()));
        createServiceTicketQuery->bind(":audience",
                                       static_cast<int>(audience));
        createServiceTicketQuery->bind(":target_server_id", targetServerID);
        createServiceTicketQuery->bind(":expires_at", expiresAt);

        int changedRowCount{createServiceTicketQuery->exec()};
        createServiceTicketQuery->reset();

        if (changedRowCount == 0) {
            return CreateServiceTicketResult::SessionUnavailable;
        }

        return CreateServiceTicketResult::Success;
    } catch (std::exception& e) {
        createServiceTicketQuery->tryReset();
        LOG_ERROR("Failed to create service ticket: %s", e.what());
        return CreateServiceTicketResult::DatabaseError;
    }
}

Database::ConsumedServiceTicketInfo Database::consumeServiceTicket(
    const std::string& tokenHash, ServiceTicketAudience audience,
    Sint64 targetServerID)
{
    ConsumedServiceTicketInfo ticketInfo{};

    try {
        consumeServiceTicketQuery->bind(":token_hash", tokenHash.data(),
                                        static_cast<int>(tokenHash.size()));
        consumeServiceTicketQuery->bind(":audience",
                                        static_cast<int>(audience));
        consumeServiceTicketQuery->bind(":target_server_id", targetServerID);

        if (!(consumeServiceTicketQuery->executeStep())) {
            consumeServiceTicketQuery->reset();
            ticketInfo.result
                = ConsumedServiceTicketInfo::Result::TicketNotFound;
            return ticketInfo;
        }

        ticketInfo.accountSessionID
            = consumeServiceTicketQuery->getColumn(0).getInt64();
        ticketInfo.accountID
            = consumeServiceTicketQuery->getColumn(1).getInt64();
        ticketInfo.accountStatus
            = consumeServiceTicketQuery->getColumn(2).getString();
        ticketInfo.consumedAt
            = consumeServiceTicketQuery->getColumn(3).getInt64();

        consumeServiceTicketQuery->reset();
        ticketInfo.result = ConsumedServiceTicketInfo::Result::Success;
        return ticketInfo;
    } catch (std::exception& e) {
        consumeServiceTicketQuery->tryReset();
        LOG_ERROR("Failed to consume service ticket: %s", e.what());
        ticketInfo.result = ConsumedServiceTicketInfo::Result::DatabaseError;
        return ticketInfo;
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

        // Login sessions.
        if (!database.tableExists("account_sessions")) {
            database.exec(R"(
                CREATE TABLE account_sessions
                (
                    session_id          INTEGER PRIMARY KEY,

                    account_id          INTEGER NOT NULL
                        REFERENCES accounts(account_id)
                        ON DELETE CASCADE,

                    token_hash          BLOB NOT NULL UNIQUE
                        CHECK (length(token_hash) = 32),

                    created_at          INTEGER NOT NULL,
                    last_used_at        INTEGER NOT NULL,
                    idle_expires_at     INTEGER NOT NULL,
                    absolute_expires_at INTEGER NOT NULL,

                    revoked_at          INTEGER,

                    CHECK (last_used_at >= created_at),
                    CHECK (idle_expires_at > last_used_at),
                    CHECK (absolute_expires_at >= idle_expires_at),
                    CHECK (
                        revoked_at IS NULL OR revoked_at >= created_at
                    )
                ) STRICT;

                CREATE INDEX account_sessions_account_id_idx
                    ON account_sessions(account_id)
                    WHERE revoked_at IS NULL;

                CREATE INDEX account_sessions_idle_expires_at_idx
                    ON account_sessions(idle_expires_at);

                CREATE INDEX account_sessions_absolute_expires_at_idx
                    ON account_sessions(absolute_expires_at);
            )");
        }

        // Single-use tickets for connecting to a service server.
        if (!database.tableExists("service_tickets")) {
            database.exec(R"(
                CREATE TABLE service_tickets
                (
                    ticket_id          INTEGER PRIMARY KEY,

                    token_hash         BLOB NOT NULL UNIQUE
                        CHECK (length(token_hash) = 32),

                    account_session_id INTEGER NOT NULL
                        REFERENCES account_sessions(session_id)
                        ON DELETE CASCADE,

                    audience           INTEGER NOT NULL
                        CHECK (audience IN (0, 1)),

                    target_server_id   INTEGER NOT NULL,

                    created_at         INTEGER NOT NULL,
                    expires_at         INTEGER NOT NULL,
                    consumed_at        INTEGER,
                    revoked_at         INTEGER,

                    CHECK (expires_at > created_at),
                    CHECK (
                        consumed_at IS NULL OR consumed_at >= created_at
                    ),
                    CHECK (
                        revoked_at IS NULL OR revoked_at >= created_at
                    ),
                    CHECK (consumed_at IS NULL OR revoked_at IS NULL)
                ) STRICT;

                CREATE INDEX service_tickets_account_session_id_idx
                    ON service_tickets(account_session_id)
                    WHERE consumed_at IS NULL AND revoked_at IS NULL;

                CREATE INDEX service_tickets_expires_at_idx
                    ON service_tickets(expires_at);
            )");
        }
    } catch (std::exception& e) {
        LOG_ERROR("Failed to init table: %s", e.what());
    }
}

} // namespace AccountServer
} // namespace AM
