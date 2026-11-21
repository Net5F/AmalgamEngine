#pragma once

#include "AccountDefs.h"
#include "AccountData.h"
#include "AccountRoleDefs.h"
#include "ModerationDefs.h"
#include "SQLiteCpp/SQLiteCpp.h"
#include <SDL3/SDL_stdinc.h>
#include <memory>
#include <mutex>
#include <string>

namespace AM
{
namespace AccountServer
{

/**
 * Interface for interacting with the account database (Accounts.db).
 *
 * We use the database to persist accounts, account sessions, service
 * tickets, characters (each account's AccountData), bans, and the moderation
 * log.
 *
 * Secrets (recovery keys, session tokens, service tickets) are never stored
 * directly. Instead, we store their hashes (see CryptoHelpers::hashSecret()).
 *
 * Note: This class is thread-safe. Each public function holds databaseMutex
 *       for its duration, so calls from multiple database workers are
 *       serialized.
 */
class Database
{
public:
    /**
     * Opens the Accounts.db next to our executable.
     */
    Database();

    /**
     * Opens the database at the given path. Creates it if it doesn't exist.
     *
     * Note: Another process (e.g. AccountServer --set-role) may use the same
     *       file. If it's holding a lock, we wait up to BUSY_TIMEOUT_MS
     *       for it.
     */
    explicit Database(const std::string& databasePath);

    //-------------------------------------------------------------------------
    // Accounts
    //-------------------------------------------------------------------------
    enum class RegisterResult { Success, UsernameUnavailable, DatabaseError };
    /**
     * Registers an account using an already-hashed password and recovery key.
     * The account is given the default role.
     *
     * @return UsernameUnavailable if the normalized username is already
     *         registered, DatabaseError if either insert fails, otherwise
     *         Success.
     */
    RegisterResult registerAccount(const std::string& username,
                                   const std::string& passwordHash,
                                   const std::string& recoveryKeyHash);

    struct AccountLoginInfo {
        enum class Result { Success, AccountNotFound, DatabaseError };

        Result result{Result::DatabaseError};
        Sint64 accountID{0};
        std::string passwordHash{};
        std::string status{};
        /** True if the account has a ban that's in effect. */
        bool isBanned{false};
        /** If isBanned, this is the Unix timestamp at which the ban will
            expire. 0 if the ban is permanent. */
        Sint64 banExpiresAt{0};
    };
    /**
     * Gets the information needed to attempt a login for the account with the
     * given username.
     *
     * @return An AccountLoginInfo with result set to AccountNotFound if no
     *         matching normalized username exists, DatabaseError if the query
     *         fails, or Success with the account fields populated.
     */
    AccountLoginInfo getAccountLoginInfo(const std::string& username);

    enum class RecoverAccountResult {
        Success,
        InvalidAccountDetails,
        DatabaseError
    };
    /**
     * Atomically consumes an account's recovery key, replaces its password,
     * issues it a new recovery key, and revokes all of its sessions.
     *
     * @param username The account's username (normalized before lookup).
     * @param recoveryKeyHash The 32-byte hash of the recovery key to consume.
     * @param newPasswordHash The already-hashed new password.
     * @param newRecoveryKeyHash The 32-byte hash of the replacement key.
     * @return InvalidAccountDetails if no active account with the given
     *         username has a matching unused, unrevoked recovery key,
     *         DatabaseError if any step fails, otherwise Success.
     */
    RecoverAccountResult recoverAccount(const std::string& username,
                                        const std::string& recoveryKeyHash,
                                        const std::string& newPasswordHash,
                                        const std::string& newRecoveryKeyHash);

    //-------------------------------------------------------------------------
    // Sessions
    //-------------------------------------------------------------------------
    enum class CreateSessionResult {
        Success,
        AccountUnavailable,
        DatabaseError
    };
    /**
     * Creates a session for an active, unbanned account.
     *
     * @param accountID The account that owns the session.
     * @param tokenHash The 32-byte hash of the session token.
     * @param idleExpiresAt The Unix timestamp at which the session expires due
     *        to inactivity.
     * @param absoluteExpiresAt The Unix timestamp at which the session expires
     *        regardless of activity.
     */
    CreateSessionResult createSession(Sint64 accountID,
                                      const std::string& tokenHash,
                                      Sint64 idleExpiresAt,
                                      Sint64 absoluteExpiresAt);

    struct AccountSessionInfo {
        enum class Result { Success, SessionNotFound, DatabaseError };

        Result result{Result::DatabaseError};
        Sint64 sessionID{0};
        Sint64 accountID{0};
        Sint64 createdAt{0};
        Sint64 lastUsedAt{0};
        Sint64 idleExpiresAt{0};
        Sint64 absoluteExpiresAt{0};
    };
    /**
     * Validates and refreshes an active session by its 32-byte token hash.
     *
     * Validation and refresh are performed atomically. Sessions that are
     * revoked, idle-expired, absolute-expired, or owned by an inactive account
     * are not returned. On success, lastUsedAt is updated and idleExpiresAt is
     * extended by idleTimeoutS without exceeding absoluteExpiresAt.
     */
    AccountSessionInfo validateSession(const std::string& tokenHash,
                                       Sint64 idleTimeoutS);

    enum class RevokeSessionResult { Success, SessionNotFound, DatabaseError };
    /**
     * Revokes the session with the given 32-byte token hash.
     */
    RevokeSessionResult revokeSession(const std::string& tokenHash);

    enum class RevokeAllSessionsResult { Success, DatabaseError };
    /**
     * Revokes every non-revoked session belonging to an account.
     */
    RevokeAllSessionsResult revokeAllSessions(Sint64 accountID);

    enum class DeleteExpiredSessionsResult { Success, DatabaseError };
    /**
     * Deletes all expired service tickets, and all revoked or expired
     * sessions (along with their remaining tickets).
     *
     * Every query already ignores these rows, so this only reclaims space.
     */
    DeleteExpiredSessionsResult deleteExpiredSessions();

    //-------------------------------------------------------------------------
    // Service tickets
    //-------------------------------------------------------------------------
    enum class CreateServiceTicketResult {
        Success,
        SessionUnavailable,
        DatabaseError
    };
    /**
     * Creates a service ticket linked to an active account session.
     *
     * @param accountSessionID The session that owns the ticket.
     * @param tokenHash The 32-byte hash of the service ticket.
     * @param audience The service that may consume the ticket.
     * @param expiresAt The Unix timestamp at which the ticket expires.
     */
    CreateServiceTicketResult
        createServiceTicket(Sint64 accountSessionID,
                            const std::string& tokenHash,
                            ServiceTicketAudience audience, Sint64 expiresAt);

    struct ConsumedServiceTicketInfo {
        enum class Result { Success, TicketNotFound, DatabaseError };

        Result result{Result::DatabaseError};
        Sint64 accountSessionID{0};
        Sint64 accountID{0};
        std::string accountStatus{};
        std::string username{};
        std::string roleID{};
        Sint64 consumedAt{0};
    };
    /**
     * Atomically validates and consumes a service ticket.
     *
     * The ticket must be unconsumed, unrevoked, unexpired, and bound to the
     * given audience. Its account session and account must also remain valid.
     */
    ConsumedServiceTicketInfo
        consumeServiceTicket(const std::string& tokenHash,
                             ServiceTicketAudience audience);

    //-------------------------------------------------------------------------
    // Characters
    //-------------------------------------------------------------------------
    enum class GetAccountDataResult { Success, NotFound, DatabaseError };
    /**
     * If the given account has saved AccountData, copies it into
     * outAccountData.
     *
     * @return NotFound if the account has no saved data (e.g. it's never
     *         joined a world), DatabaseError if the query failed, else
     *         Success.
     *         Don't treat DatabaseError as NotFound. Doing so would give the
     *         account fresh data, which would overwrite its real data on the
     *         next save.
     */
    GetAccountDataResult getAccountData(Sint64 accountID,
                                        AccountData& outAccountData);

    enum class SaveAccountDataResult {
        Success,
        AccountNotFound,
        DatabaseError
    };
    /**
     * Adds or overwrites the given account's AccountData.
     *
     * @return AccountNotFound if the account doesn't exist, DatabaseError if
     *         the query failed, else Success.
     */
    SaveAccountDataResult saveAccountData(Sint64 accountID,
                                          const AccountData& accountData);

    //-------------------------------------------------------------------------
    // Moderation
    //-------------------------------------------------------------------------
    struct ModerationInfo {
        /** The account that's taking the action. */
        Sint64 actorAccountID{0};
        AccountModerationAction action{AccountModerationAction::Ban};
        std::string targetUsername{};
        /** (SetRole only) The role to give the target. */
        std::string roleID{};
        /** (Ban only) How long the ban lasts. 0 means permanent. */
        Sint64 durationS{0};
        std::string reason{};
    };
    struct ModerationOutcome {
        ModerationResult result{ModerationResult::InternalError};
        /** If result == Success, the target's account ID. */
        Sint64 targetAccountID{0};
        /** If result == Success, the target's username as registered. */
        std::string targetUsername{};
        /** If result == Success and action == SetRole, the target's new role
            ID. */
        std::string roleID{};
    };
    /**
     * Atomically checks that the actor may take the given action against the
     * target, applies it, and records it in the moderation log.
     *
     * Roles are read from the database, so the result reflects any role
     * changes that services haven't heard about yet.
     *
     * Ban: Lifts any existing ban, adds the new one, and revokes all of the
     *      target's sessions.
     * Unban: Lifts the target's active ban.
     * SetRole: Changes the target's role.
     */
    ModerationOutcome moderate(const ModerationInfo& info);

    struct ConsoleSetRoleResult {
        enum class Result { Success, AccountNotFound, DatabaseError };

        Result result{Result::DatabaseError};
        /** If result == Success, the account's username as registered. */
        std::string username{};
        /** If result == Success, the role that the account held before. */
        std::string previousRoleID{};
    };
    /**
     * Changes the given account's role without any permission checks, and
     * records it in the moderation log as being done by the console.
     *
     * Used by the --set-role command line option, e.g. to create the first
     * SuperAdmin.
     *
     * @param roleID Must be a valid role ID.
     */
    ConsoleSetRoleResult setRoleFromConsole(const std::string& username,
                                            const std::string& roleID);

protected:
    /** How long we'll wait for another connection's lock before failing. */
    static constexpr int BUSY_TIMEOUT_MS{5000};

    /** The name that the moderation log uses for console actions. */
    static constexpr const char* CONSOLE_ACTOR_NAME{"console"};

    /**
     * Creates our tables in Accounts.db, if they don't already exist.
     */
    void initTables();

    /**
     * Returns the permission that the given action requires.
     */
    AccountPermission getRequiredPermission(AccountModerationAction action);

    /**
     * Returns the name that the moderation log uses for the given action.
     */
    const char* getLogActionName(AccountModerationAction action);

    /**
     * Adds an entry to the moderation log.
     * Must be called while holding databaseMutex.
     *
     * @param actorAccountID The acting account, or 0 for the console.
     */
    void insertModerationLog(Sint64 actorAccountID,
                             const std::string& actorName,
                             Sint64 targetAccountID,
                             const std::string& targetName,
                             AccountModerationAction action,
                             const std::string& details,
                             const std::string& reason);

    /** Serializes access to database and our pre-built queries. */
    std::mutex databaseMutex;

    /** File-backed database, storing account data. */
    SQLite::Database database;

    // Pre-built queries
    std::unique_ptr<SQLite::Statement> registerAccountQuery;
    std::unique_ptr<SQLite::Statement> insertRecoveryKeyQuery;
    std::unique_ptr<SQLite::Statement> getAccountLoginInfoQuery;
    std::unique_ptr<SQLite::Statement> consumeRecoveryKeyQuery;
    std::unique_ptr<SQLite::Statement> updatePasswordQuery;
    std::unique_ptr<SQLite::Statement> createSessionQuery;
    std::unique_ptr<SQLite::Statement> validateSessionQuery;
    std::unique_ptr<SQLite::Statement> revokeSessionQuery;
    std::unique_ptr<SQLite::Statement> revokeAllSessionsQuery;
    std::unique_ptr<SQLite::Statement> createServiceTicketQuery;
    std::unique_ptr<SQLite::Statement> consumeServiceTicketQuery;
    std::unique_ptr<SQLite::Statement> getAccountDataQuery;
    std::unique_ptr<SQLite::Statement> saveAccountDataQuery;
    std::unique_ptr<SQLite::Statement> deleteExpiredServiceTicketsQuery;
    std::unique_ptr<SQLite::Statement> deleteExpiredSessionsQuery;
    std::unique_ptr<SQLite::Statement> getAccountByIDQuery;
    std::unique_ptr<SQLite::Statement> getAccountByUsernameQuery;
    std::unique_ptr<SQLite::Statement> setRoleQuery;
    std::unique_ptr<SQLite::Statement> liftBansQuery;
    std::unique_ptr<SQLite::Statement> insertBanQuery;
    std::unique_ptr<SQLite::Statement> insertModerationLogQuery;
};

} // End namespace AccountServer
} // End namespace AM
