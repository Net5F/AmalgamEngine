#include "Database.h"
#include "Paths.h"
#include "AccountRoleHelpers.h"
#include "AccountRoleDefs.h"
#include "Log.h"
#include <mutex>
#include <string>

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
: Database(Paths::BASE_PATH + "Accounts.db")
{
}

Database::Database(const std::string& databasePath)
: database{databasePath, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE,
           BUSY_TIMEOUT_MS}
, registerAccountQuery{nullptr}
, insertRecoveryKeyQuery{nullptr}
, getAccountLoginInfoQuery{nullptr}
, consumeRecoveryKeyQuery{nullptr}
, updatePasswordQuery{nullptr}
, createSessionQuery{nullptr}
, validateSessionQuery{nullptr}
, revokeSessionQuery{nullptr}
, revokeAllSessionsQuery{nullptr}
, createServiceTicketQuery{nullptr}
, consumeServiceTicketQuery{nullptr}
, getAccountDataQuery{nullptr}
, saveAccountDataQuery{nullptr}
, deleteExpiredServiceTicketsQuery{nullptr}
, deleteExpiredSessionsQuery{nullptr}
, getAccountByIDQuery{nullptr}
, getAccountByUsernameQuery{nullptr}
, setRoleQuery{nullptr}
, liftBansQuery{nullptr}
, insertBanQuery{nullptr}
, insertModerationLogQuery{nullptr}
{
    // Enforce our REFERENCES constraints, including their ON DELETE CASCADE.
    // Note: SQLite leaves this off by default, and it must be set per
    //       connection (outside of any transaction).
    database.exec("PRAGMA foreign_keys = ON");

    // If any of our tables don't exist in Accounts.db, initialize them.
    initTables();

    // Note: We build these queries after initTables() because they'll
    //       segfault if there's no DB with the expected fields.
    registerAccountQuery = std::make_unique<SQLite::Statement>(database, R"(
            INSERT INTO accounts
                (username, normalized_username, password_hash, role,
                 created_at, updated_at)
            VALUES
                (:username, lower(:username), :password_hash, :role,
                 unixepoch(), unixepoch())
            ON CONFLICT(normalized_username) DO NOTHING
        )");

    insertRecoveryKeyQuery = std::make_unique<SQLite::Statement>(database, R"(
            INSERT INTO account_recovery_keys
                (account_id, key_hash, created_at)
            VALUES
                (:account_id, :key_hash, unixepoch())
        )");

    getAccountLoginInfoQuery = std::make_unique<SQLite::Statement>(database, R"(
            SELECT account_id, password_hash, status,
                EXISTS (
                    SELECT 1
                    FROM account_bans
                    WHERE account_bans.account_id = accounts.account_id
                      AND lifted_at IS NULL
                      AND (expires_at IS NULL OR expires_at > unixepoch())
                ),
                (
                    -- The latest-expiring active ban. Null if permanent.
                    SELECT expires_at
                    FROM account_bans
                    WHERE account_bans.account_id = accounts.account_id
                      AND lifted_at IS NULL
                      AND (expires_at IS NULL OR expires_at > unixepoch())
                    ORDER BY (expires_at IS NULL) DESC, expires_at DESC
                    LIMIT 1
                )
            FROM accounts
            WHERE normalized_username = lower(:username)
            LIMIT 1
        )");

    consumeRecoveryKeyQuery = std::make_unique<SQLite::Statement>(database, R"(
            UPDATE account_recovery_keys
            SET used_at = unixepoch()
            WHERE key_hash = :key_hash
              AND used_at IS NULL
              AND revoked_at IS NULL
              AND account_id = (
                  SELECT account_id
                  FROM accounts
                  WHERE normalized_username = lower(:username)
                    AND status = 'active'
              )
            RETURNING account_id
        )");

    updatePasswordQuery = std::make_unique<SQLite::Statement>(database, R"(
            UPDATE accounts
            SET password_hash = :password_hash,
                updated_at = unixepoch()
            WHERE account_id = :account_id
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
              AND NOT EXISTS (
                  SELECT 1
                  FROM account_bans
                  WHERE account_bans.account_id = accounts.account_id
                    AND lifted_at IS NULL
                    AND (expires_at IS NULL OR expires_at > unixepoch())
              )
        )");

    validateSessionQuery = std::make_unique<SQLite::Statement>(database, R"(
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

    revokeAllSessionsQuery = std::make_unique<SQLite::Statement>(database, R"(
            UPDATE account_sessions
            SET revoked_at = unixepoch()
            WHERE account_id = :account_id
              AND revoked_at IS NULL
        )");

    createServiceTicketQuery = std::make_unique<SQLite::Statement>(database, R"(
            INSERT INTO service_tickets
                (token_hash, account_session_id, audience, created_at,
                 expires_at)
            SELECT
                :token_hash, sessions.session_id, :audience, unixepoch(),
                :expires_at
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
                (
                    SELECT accounts.username
                    FROM account_sessions AS sessions
                    JOIN accounts
                      ON accounts.account_id = sessions.account_id
                    WHERE sessions.session_id
                              = service_tickets.account_session_id
                ),
                (
                    SELECT accounts.role
                    FROM account_sessions AS sessions
                    JOIN accounts
                      ON accounts.account_id = sessions.account_id
                    WHERE sessions.session_id
                              = service_tickets.account_session_id
                ),
                consumed_at
        )");

    getAccountDataQuery = std::make_unique<SQLite::Statement>(database, R"(
            SELECT serialized_engine_components, serialized_project_components
            FROM characters
            WHERE account_id = :account_id
        )");

    // Note: The SELECT only produces a row if the account exists, so saves
    //       for unknown accounts change nothing.
    saveAccountDataQuery = std::make_unique<SQLite::Statement>(database, R"(
            INSERT INTO characters
                (account_id, serialized_engine_components,
                 serialized_project_components, created_at, updated_at)
            SELECT
                account_id, :engine_components, :project_components,
                unixepoch(), unixepoch()
            FROM accounts
            WHERE account_id = :account_id
            ON CONFLICT(account_id) DO UPDATE SET
                serialized_engine_components
                    = excluded.serialized_engine_components,
                serialized_project_components
                    = excluded.serialized_project_components,
                updated_at = excluded.updated_at
        )");

    // Note: Consumed and revoked tickets are kept until they expire, which is
    //       at most SERVICE_TICKET_LIFETIME_S later.
    deleteExpiredServiceTicketsQuery
        = std::make_unique<SQLite::Statement>(database, R"(
            DELETE FROM service_tickets
            WHERE expires_at <= unixepoch()
        )");

    // Note: idle_expires_at is clamped to absolute_expires_at on refresh, so
    //       checking it covers both expirations.
    // Note: Deleting a session cascades to its service tickets.
    deleteExpiredSessionsQuery
        = std::make_unique<SQLite::Statement>(database, R"(
            DELETE FROM account_sessions
            WHERE revoked_at IS NOT NULL
               OR idle_expires_at <= unixepoch()
        )");

    getAccountByIDQuery = std::make_unique<SQLite::Statement>(database, R"(
            SELECT username, role
            FROM accounts
            WHERE account_id = :account_id
        )");

    getAccountByUsernameQuery
        = std::make_unique<SQLite::Statement>(database, R"(
            SELECT account_id, username, role
            FROM accounts
            WHERE normalized_username = lower(:username)
        )");

    setRoleQuery = std::make_unique<SQLite::Statement>(database, R"(
            UPDATE accounts
            SET role = :role,
                updated_at = unixepoch()
            WHERE account_id = :account_id
        )");

    liftBansQuery = std::make_unique<SQLite::Statement>(database, R"(
            UPDATE account_bans
            SET lifted_at = unixepoch(),
                lifted_by = :lifted_by
            WHERE account_id = :account_id
              AND lifted_at IS NULL
              AND (expires_at IS NULL OR expires_at > unixepoch())
        )");

    // Note: A duration of 0 means the ban is permanent.
    insertBanQuery = std::make_unique<SQLite::Statement>(database, R"(
            INSERT INTO account_bans
                (account_id, banned_by, reason, created_at, expires_at)
            VALUES
                (:account_id, :banned_by, :reason, unixepoch(),
                 CASE WHEN :duration_s > 0 THEN unixepoch() + :duration_s
                      ELSE NULL END)
        )");

    insertModerationLogQuery = std::make_unique<SQLite::Statement>(database, R"(
            INSERT INTO moderation_log
                (actor_account_id, actor_name, target_account_id, target_name,
                 action, details, reason, created_at)
            VALUES
                (:actor_account_id, :actor_name, :target_account_id,
                 :target_name, :action, :details, :reason, unixepoch())
        )");
}

Database::RegisterResult
    Database::registerAccount(const std::string& username,
                              const std::string& passwordHash,
                              const std::string& recoveryKeyHash)
{
    std::scoped_lock lock{databaseMutex};

    try {
        SQLite::Transaction transaction{database};

        // Insert the username/password.
        registerAccountQuery->bind(":username", username);
        registerAccountQuery->bind(":password_hash", passwordHash);
        registerAccountQuery->bind(
            ":role", std::string{AccountRoleDefs::DEFAULT_ROLE_ID});

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
    std::scoped_lock lock{databaseMutex};

    AccountLoginInfo loginInfo{};

    try {
        getAccountLoginInfoQuery->bind(":username", username);

        if (!(getAccountLoginInfoQuery->executeStep())) {
            getAccountLoginInfoQuery->reset();
            loginInfo.result = AccountLoginInfo::Result::AccountNotFound;
            return loginInfo;
        }

        loginInfo.accountID = getAccountLoginInfoQuery->getColumn(0).getInt64();
        loginInfo.passwordHash
            = getAccountLoginInfoQuery->getColumn(1).getString();
        loginInfo.status = getAccountLoginInfoQuery->getColumn(2).getString();
        loginInfo.isBanned
            = (getAccountLoginInfoQuery->getColumn(3).getInt() != 0);
        SQLite::Column banExpiresAtColumn{
            getAccountLoginInfoQuery->getColumn(4)};
        loginInfo.banExpiresAt
            = (banExpiresAtColumn.isNull() ? 0 : banExpiresAtColumn.getInt64());

        getAccountLoginInfoQuery->reset();
        loginInfo.result = AccountLoginInfo::Result::Success;
    } catch (std::exception& e) {
        getAccountLoginInfoQuery->tryReset();
        LOG_ERROR("Failed to get account login info: %s", e.what());
        loginInfo.result = AccountLoginInfo::Result::DatabaseError;
    }

    return loginInfo;
}

Database::RecoverAccountResult Database::recoverAccount(
    const std::string& username, const std::string& recoveryKeyHash,
    const std::string& newPasswordHash, const std::string& newRecoveryKeyHash)
{
    std::scoped_lock lock{databaseMutex};

    try {
        SQLite::Transaction transaction{database};

        // Consume the recovery key. This also confirms that it belongs to the
        // given active account.
        consumeRecoveryKeyQuery->bind(":username", username);
        consumeRecoveryKeyQuery->bind(":key_hash", recoveryKeyHash.data(),
                                      static_cast<int>(recoveryKeyHash.size()));

        if (!(consumeRecoveryKeyQuery->executeStep())) {
            consumeRecoveryKeyQuery->reset();
            return RecoverAccountResult::InvalidAccountDetails;
        }

        Sint64 accountID{consumeRecoveryKeyQuery->getColumn(0).getInt64()};
        consumeRecoveryKeyQuery->reset();

        // Replace the password.
        updatePasswordQuery->bind(":account_id", accountID);
        updatePasswordQuery->bind(":password_hash", newPasswordHash);
        updatePasswordQuery->exec();
        updatePasswordQuery->reset();

        // Insert the replacement recovery key.
        insertRecoveryKeyQuery->bind(":account_id", accountID);
        insertRecoveryKeyQuery->bind(
            ":key_hash", newRecoveryKeyHash.data(),
            static_cast<int>(newRecoveryKeyHash.size()));
        insertRecoveryKeyQuery->exec();
        insertRecoveryKeyQuery->reset();

        // Log out everywhere, in case the account was compromised.
        // Note: Outstanding service tickets become unusable along with their
        //       sessions.
        revokeAllSessionsQuery->bind(":account_id", accountID);
        revokeAllSessionsQuery->exec();
        revokeAllSessionsQuery->reset();

        transaction.commit();
        return RecoverAccountResult::Success;
    } catch (std::exception& e) {
        // Ensure the pre-built statements are reusable after a failure.
        consumeRecoveryKeyQuery->tryReset();
        updatePasswordQuery->tryReset();
        insertRecoveryKeyQuery->tryReset();
        revokeAllSessionsQuery->tryReset();
        LOG_ERROR("Failed to recover account: %s", e.what());
        return RecoverAccountResult::DatabaseError;
    }
}

Database::CreateSessionResult
    Database::createSession(Sint64 accountID, const std::string& tokenHash,
                            Sint64 idleExpiresAt, Sint64 absoluteExpiresAt)
{
    std::scoped_lock lock{databaseMutex};

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
    Database::validateSession(const std::string& tokenHash, Sint64 idleTimeoutS)
{
    std::scoped_lock lock{databaseMutex};

    AccountSessionInfo sessionInfo{};

    try {
        validateSessionQuery->bind(":token_hash", tokenHash.data(),
                                   static_cast<int>(tokenHash.size()));
        validateSessionQuery->bind(":idle_timeout_s", idleTimeoutS);

        if (!(validateSessionQuery->executeStep())) {
            validateSessionQuery->reset();
            sessionInfo.result = AccountSessionInfo::Result::SessionNotFound;
            return sessionInfo;
        }

        sessionInfo.sessionID = validateSessionQuery->getColumn(0).getInt64();
        sessionInfo.accountID = validateSessionQuery->getColumn(1).getInt64();
        sessionInfo.createdAt = validateSessionQuery->getColumn(2).getInt64();
        sessionInfo.lastUsedAt = validateSessionQuery->getColumn(3).getInt64();
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
    std::scoped_lock lock{databaseMutex};

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

Database::RevokeAllSessionsResult Database::revokeAllSessions(Sint64 accountID)
{
    std::scoped_lock lock{databaseMutex};

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

Database::DeleteExpiredSessionsResult Database::deleteExpiredSessions()
{
    std::scoped_lock lock{databaseMutex};

    try {
        SQLite::Transaction transaction{database};

        int deletedTicketCount{deleteExpiredServiceTicketsQuery->exec()};
        deleteExpiredServiceTicketsQuery->reset();

        int deletedSessionCount{deleteExpiredSessionsQuery->exec()};
        deleteExpiredSessionsQuery->reset();

        transaction.commit();

        if ((deletedSessionCount > 0) || (deletedTicketCount > 0)) {
            LOG_INFO("Deleted %d expired sessions and %d expired service "
                     "tickets.",
                     deletedSessionCount, deletedTicketCount);
        }
        return DeleteExpiredSessionsResult::Success;
    } catch (std::exception& e) {
        deleteExpiredServiceTicketsQuery->tryReset();
        deleteExpiredSessionsQuery->tryReset();
        LOG_ERROR("Failed to delete expired sessions: %s", e.what());
        return DeleteExpiredSessionsResult::DatabaseError;
    }
}

Database::CreateServiceTicketResult Database::createServiceTicket(
    Sint64 accountSessionID, const std::string& tokenHash,
    ServiceTicketAudience audience, Sint64 expiresAt)
{
    std::scoped_lock lock{databaseMutex};

    try {
        createServiceTicketQuery->bind(":account_session_id", accountSessionID);
        createServiceTicketQuery->bind(":token_hash", tokenHash.data(),
                                       static_cast<int>(tokenHash.size()));
        createServiceTicketQuery->bind(":audience", static_cast<int>(audience));
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

Database::ConsumedServiceTicketInfo
    Database::consumeServiceTicket(const std::string& tokenHash,
                                   ServiceTicketAudience audience)
{
    std::scoped_lock lock{databaseMutex};

    ConsumedServiceTicketInfo ticketInfo{};

    try {
        consumeServiceTicketQuery->bind(":token_hash", tokenHash.data(),
                                        static_cast<int>(tokenHash.size()));
        consumeServiceTicketQuery->bind(":audience",
                                        static_cast<int>(audience));

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
        ticketInfo.username
            = consumeServiceTicketQuery->getColumn(3).getString();
        ticketInfo.roleID = consumeServiceTicketQuery->getColumn(4).getString();
        ticketInfo.consumedAt
            = consumeServiceTicketQuery->getColumn(5).getInt64();

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

Database::GetAccountDataResult
    Database::getAccountData(Sint64 accountID, AccountData& outAccountData)
{
    std::scoped_lock lock{databaseMutex};

    try {
        getAccountDataQuery->bind(":account_id", accountID);

        if (!(getAccountDataQuery->executeStep())) {
            getAccountDataQuery->reset();
            return GetAccountDataResult::NotFound;
        }

        auto copyBlob = [&](int columnIndex, std::vector<Uint8>& outBlob) {
            SQLite::Column column{getAccountDataQuery->getColumn(columnIndex)};
            const Uint8* data{static_cast<const Uint8*>(column.getBlob())};
            outBlob.assign(data, data + column.getBytes());
        };
        copyBlob(0, outAccountData.engineComponents);
        copyBlob(1, outAccountData.projectComponents);

        getAccountDataQuery->reset();
        return GetAccountDataResult::Success;
    } catch (std::exception& e) {
        getAccountDataQuery->tryReset();
        LOG_ERROR("Failed to get account data: %s", e.what());
        return GetAccountDataResult::DatabaseError;
    }
}

Database::SaveAccountDataResult
    Database::saveAccountData(Sint64 accountID, const AccountData& accountData)
{
    std::scoped_lock lock{databaseMutex};

    // Note: Binding a null pointer stores NULL instead of an empty blob, so
    //       empty vectors (whose data() may be null) bind a dummy pointer.
    static constexpr Uint8 EMPTY_BLOB{0};
    auto bindBlob = [&](const char* name, const std::vector<Uint8>& blob) {
        saveAccountDataQuery->bind(name,
                                   blob.empty() ? &EMPTY_BLOB : blob.data(),
                                   static_cast<int>(blob.size()));
    };

    try {
        saveAccountDataQuery->bind(":account_id", accountID);
        bindBlob(":engine_components", accountData.engineComponents);
        bindBlob(":project_components", accountData.projectComponents);

        int changedRowCount{saveAccountDataQuery->exec()};
        saveAccountDataQuery->reset();

        if (changedRowCount == 0) {
            return SaveAccountDataResult::AccountNotFound;
        }

        return SaveAccountDataResult::Success;
    } catch (std::exception& e) {
        saveAccountDataQuery->tryReset();
        LOG_ERROR("Failed to save account data: %s", e.what());
        return SaveAccountDataResult::DatabaseError;
    }
}

Database::ModerationOutcome Database::moderate(const ModerationInfo& info)
{
    std::scoped_lock lock{databaseMutex};

    ModerationOutcome outcome{};

    try {
        // Note: If we return before committing, the transaction is rolled
        //       back.
        SQLite::Transaction transaction{database};

        // Get the actor's role, and check that it permits this action.
        getAccountByIDQuery->bind(":account_id", info.actorAccountID);
        if (!(getAccountByIDQuery->executeStep())) {
            // The actor's account no longer exists.
            getAccountByIDQuery->reset();
            outcome.result = ModerationResult::NotPermitted;
            return outcome;
        }
        std::string actorName{getAccountByIDQuery->getColumn(0).getString()};
        const AccountRoleDef& actorRole{AccountRoleHelpers::getRoleOrDefault(
            getAccountByIDQuery->getColumn(1).getString())};
        getAccountByIDQuery->reset();

        if (!(actorRole.hasPermission(getRequiredPermission(info.action)))) {
            outcome.result = ModerationResult::NotPermitted;
            return outcome;
        }

        // Get the target, and check that the actor outranks them.
        getAccountByUsernameQuery->bind(":username", info.targetUsername);
        if (!(getAccountByUsernameQuery->executeStep())) {
            getAccountByUsernameQuery->reset();
            outcome.result = ModerationResult::TargetNotFound;
            return outcome;
        }
        Sint64 targetAccountID{
            getAccountByUsernameQuery->getColumn(0).getInt64()};
        std::string targetName{
            getAccountByUsernameQuery->getColumn(1).getString()};
        const AccountRoleDef& targetRole{AccountRoleHelpers::getRoleOrDefault(
            getAccountByUsernameQuery->getColumn(2).getString())};
        getAccountByUsernameQuery->reset();

        if (!(AccountRoleHelpers::canActOn(actorRole, targetRole))) {
            outcome.result = ModerationResult::NotPermitted;
            return outcome;
        }

        // Apply the action.
        std::string details{};
        switch (info.action) {
            case AccountModerationAction::Ban: {
                // Replace any existing ban.
                liftBansQuery->bind(":account_id", targetAccountID);
                liftBansQuery->bind(":lifted_by", info.actorAccountID);
                liftBansQuery->exec();
                liftBansQuery->reset();

                insertBanQuery->bind(":account_id", targetAccountID);
                insertBanQuery->bind(":banned_by", info.actorAccountID);
                insertBanQuery->bind(":reason", info.reason);
                insertBanQuery->bind(":duration_s", info.durationS);
                insertBanQuery->exec();
                insertBanQuery->reset();

                // Log out everywhere.
                // Note: Outstanding service tickets become unusable along
                //       with their sessions.
                revokeAllSessionsQuery->bind(":account_id", targetAccountID);
                revokeAllSessionsQuery->exec();
                revokeAllSessionsQuery->reset();

                details = (info.durationS > 0)
                              ? (std::to_string(info.durationS) + "s")
                              : "permanent";
                break;
            }
            case AccountModerationAction::Unban: {
                liftBansQuery->bind(":account_id", targetAccountID);
                liftBansQuery->bind(":lifted_by", info.actorAccountID);
                int liftedCount{liftBansQuery->exec()};
                liftBansQuery->reset();

                if (liftedCount == 0) {
                    outcome.result = ModerationResult::NotBanned;
                    return outcome;
                }
                break;
            }
            case AccountModerationAction::SetRole: {
                const AccountRoleDef* newRole{
                    AccountRoleHelpers::findRole(info.roleID)};
                if (!newRole) {
                    outcome.result = ModerationResult::InvalidRole;
                    return outcome;
                }
                if (!(AccountRoleHelpers::canAssign(actorRole, *newRole))) {
                    outcome.result = ModerationResult::NotPermitted;
                    return outcome;
                }

                setRoleQuery->bind(":account_id", targetAccountID);
                setRoleQuery->bind(":role", std::string{newRole->id});
                setRoleQuery->exec();
                setRoleQuery->reset();

                details = std::string{targetRole.id} + " -> "
                          + std::string{newRole->id};
                outcome.roleID = newRole->id;
                break;
            }
            default: {
                LOG_ERROR("Unknown moderation action: %u",
                          static_cast<unsigned int>(info.action));
                outcome.result = ModerationResult::InternalError;
                return outcome;
            }
        }

        insertModerationLog(info.actorAccountID, actorName, targetAccountID,
                            targetName, info.action, details, info.reason);

        transaction.commit();

        outcome.result = ModerationResult::Success;
        outcome.targetAccountID = targetAccountID;
        outcome.targetUsername = std::move(targetName);
        return outcome;
    } catch (std::exception& e) {
        // Ensure the pre-built statements are reusable after a failure.
        getAccountByIDQuery->tryReset();
        getAccountByUsernameQuery->tryReset();
        liftBansQuery->tryReset();
        insertBanQuery->tryReset();
        revokeAllSessionsQuery->tryReset();
        setRoleQuery->tryReset();
        insertModerationLogQuery->tryReset();
        LOG_ERROR("Failed to apply moderation action: %s", e.what());
        outcome = {};
        outcome.result = ModerationResult::InternalError;
        return outcome;
    }
}

Database::ConsoleSetRoleResult
    Database::setRoleFromConsole(const std::string& username,
                                 const std::string& roleID)
{
    std::scoped_lock lock{databaseMutex};

    ConsoleSetRoleResult setRoleResult{};

    try {
        SQLite::Transaction transaction{database};

        getAccountByUsernameQuery->bind(":username", username);
        if (!(getAccountByUsernameQuery->executeStep())) {
            getAccountByUsernameQuery->reset();
            setRoleResult.result
                = ConsoleSetRoleResult::Result::AccountNotFound;
            return setRoleResult;
        }
        Sint64 accountID{getAccountByUsernameQuery->getColumn(0).getInt64()};
        setRoleResult.username
            = getAccountByUsernameQuery->getColumn(1).getString();
        setRoleResult.previousRoleID
            = getAccountByUsernameQuery->getColumn(2).getString();
        getAccountByUsernameQuery->reset();

        setRoleQuery->bind(":account_id", accountID);
        setRoleQuery->bind(":role", roleID);
        setRoleQuery->exec();
        setRoleQuery->reset();

        insertModerationLog(
            0, CONSOLE_ACTOR_NAME, accountID, setRoleResult.username,
            AccountModerationAction::SetRole,
            (setRoleResult.previousRoleID + " -> " + roleID), "");

        transaction.commit();
        setRoleResult.result = ConsoleSetRoleResult::Result::Success;
        return setRoleResult;
    } catch (std::exception& e) {
        getAccountByUsernameQuery->tryReset();
        setRoleQuery->tryReset();
        insertModerationLogQuery->tryReset();
        LOG_ERROR("Failed to set role: %s", e.what());
        setRoleResult = {};
        setRoleResult.result = ConsoleSetRoleResult::Result::DatabaseError;
        return setRoleResult;
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

                    -- The ID of a role in AccountRoleDefs.
                    -- Note: Roles are defined by the project, so we can't
                    --       constrain this to the valid IDs.
                    role                 TEXT NOT NULL
                        CHECK (length(role) BETWEEN 1 AND 32),

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

                CREATE INDEX account_recovery_keys_account_id_idx
                    ON account_recovery_keys(account_id);
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
                    ON account_sessions(account_id);

                CREATE INDEX account_sessions_idle_expires_at_idx
                    ON account_sessions(idle_expires_at);
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
                    ON service_tickets(account_session_id);

                CREATE INDEX service_tickets_expires_at_idx
                    ON service_tickets(expires_at);
            )");
        }

        // Each account's character data (its player entity's
        // PlayerPersistence::Account components). The blobs are opaque to us,
        // only the world server knows how to read them.
        // Note: Engine and project components use separate ID namespaces, so
        //       their record lists are stored in separate blobs.
        if (!database.tableExists("characters")) {
            database.exec(R"(
                CREATE TABLE characters
                (
                    account_id                    INTEGER PRIMARY KEY
                        REFERENCES accounts(account_id)
                        ON DELETE CASCADE,

                    serialized_engine_components  BLOB NOT NULL,
                    serialized_project_components BLOB NOT NULL,

                    created_at                    INTEGER NOT NULL,
                    updated_at                    INTEGER NOT NULL
                ) STRICT
            )");
        }

        // Account bans. A ban is in effect until it expires or is lifted.
        // Lifted and expired bans are kept as a history.
        if (!database.tableExists("account_bans")) {
            database.exec(R"(
                CREATE TABLE account_bans
                (
                    ban_id      INTEGER PRIMARY KEY,

                    account_id  INTEGER NOT NULL
                        REFERENCES accounts(account_id)
                        ON DELETE CASCADE,

                    -- The account that issued the ban.
                    banned_by   INTEGER
                        REFERENCES accounts(account_id)
                        ON DELETE SET NULL,

                    reason      TEXT NOT NULL,

                    created_at  INTEGER NOT NULL,
                    -- If null, the ban is permanent.
                    expires_at  INTEGER,

                    -- Set when the ban is lifted early (unbanned or
                    -- replaced by a newer ban).
                    lifted_at   INTEGER,
                    lifted_by   INTEGER
                        REFERENCES accounts(account_id)
                        ON DELETE SET NULL,

                    CHECK (expires_at IS NULL OR expires_at > created_at),
                    CHECK (lifted_at IS NULL OR lifted_at >= created_at)
                ) STRICT;

                CREATE INDEX account_bans_account_id_idx
                    ON account_bans(account_id);
            )");
        }

        // A record of every moderation action.
        // Note: Names are stored alongside IDs so that entries stay readable
        //       if an account is deleted.
        if (!database.tableExists("moderation_log")) {
            database.exec(R"(
                CREATE TABLE moderation_log
                (
                    log_id             INTEGER PRIMARY KEY,

                    -- Null if the action was taken through the console.
                    actor_account_id   INTEGER
                        REFERENCES accounts(account_id)
                        ON DELETE SET NULL,
                    actor_name         TEXT NOT NULL,

                    target_account_id  INTEGER
                        REFERENCES accounts(account_id)
                        ON DELETE SET NULL,
                    target_name        TEXT NOT NULL,

                    action             TEXT NOT NULL
                        CHECK (action IN (
                            'ban',
                            'unban',
                            'set_role'
                        )),
                    -- Action-specific info, e.g. a ban's duration or a role
                    -- change.
                    details            TEXT NOT NULL,
                    reason             TEXT NOT NULL,

                    created_at         INTEGER NOT NULL
                ) STRICT;

                CREATE INDEX moderation_log_target_account_id_idx
                    ON moderation_log(target_account_id);
            )");
        }
    } catch (std::exception& e) {
        LOG_ERROR("Failed to init table: %s", e.what());
    }
}

AccountPermission
    Database::getRequiredPermission(AccountModerationAction action)
{
    switch (action) {
        case AccountModerationAction::Ban:
        case AccountModerationAction::Unban: {
            return AccountPermission::Ban;
        }
        case AccountModerationAction::SetRole:
        default: {
            return AccountPermission::SetRole;
        }
    }
}

const char* Database::getLogActionName(AccountModerationAction action)
{
    switch (action) {
        case AccountModerationAction::Ban: {
            return "ban";
        }
        case AccountModerationAction::Unban: {
            return "unban";
        }
        case AccountModerationAction::SetRole:
        default: {
            return "set_role";
        }
    }
}

void Database::insertModerationLog(
    Sint64 actorAccountID, const std::string& actorName, Sint64 targetAccountID,
    const std::string& targetName, AccountModerationAction action,
    const std::string& details, const std::string& reason)
{
    if (actorAccountID == 0) {
        insertModerationLogQuery->bind(":actor_account_id");
    }
    else {
        insertModerationLogQuery->bind(":actor_account_id", actorAccountID);
    }
    insertModerationLogQuery->bind(":actor_name", actorName);
    insertModerationLogQuery->bind(":target_account_id", targetAccountID);
    insertModerationLogQuery->bind(":target_name", targetName);
    insertModerationLogQuery->bind(":action", getLogActionName(action));
    insertModerationLogQuery->bind(":details", details);
    insertModerationLogQuery->bind(":reason", reason);
    insertModerationLogQuery->exec();
    insertModerationLogQuery->reset();
}

} // End namespace AccountServer
} // End namespace AM
