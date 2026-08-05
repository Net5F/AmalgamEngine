#pragma once

#include "AccountDefs.h"
#include "SQLiteCpp/SQLiteCpp.h"
#include <SDL3/SDL_stdinc.h>
#include <memory>
#include <string>

namespace AM
{
namespace AccountServer
{

/**
 * Interface for interacting with the database.
 *
 * We use the database to persist item definitions, non-client entity data,
 * and tile map data as blobs.
 *
 * To avoid blocking the main loop, we first copy all of our data into an
 * in-memory database. Then, we use a separate thread to backup the in-memory
 * database to a file. See SaveSystem.h for more info.
 *
 * Note: Client entity data is persisted in the account database, not here.
 */
class Database
{
public:
    Database();

    /**
     * Begins a transaction. While a transaction is ongoing, queries will be
     * executed as normal, but they won't be permanent or visible to other 
     * connections until object.commit() is called.
     *
     * @return An RAII transaction object. If the object is destroyed without 
     * calling object.commit(), the transaction will be rolled back.
     */
    SQLite::Transaction startTransaction();

    /**
     * Overload to use a non-default behavior.
     */
    SQLite::Transaction startTransaction(SQLite::TransactionBehavior behavior);

    //-------------------------------------------------------------------------
    // Accounts
    //-------------------------------------------------------------------------
    enum class RegisterResult {
        Success,
        UsernameUnavailable,
        DatabaseError
    };
    /**
     * Registers an account using an already-hashed password and recovery key.
     *
     * @return UsernameUnavailable if the normalized username is already
     *         registered, DatabaseError if either insert fails, otherwise
     *         Success.
     */
    RegisterResult registerAccount(const std::string& username,
                                   const std::string& passwordHash,
                                   const std::string& recoveryKeyHash);

    struct AccountLoginInfo {
        enum class Result {
            Success,
            AccountNotFound,
            DatabaseError
        };

        Result result{Result::DatabaseError};
        Sint64 accountID{0};
        std::string passwordHash{};
        std::string status{};
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

    //-------------------------------------------------------------------------
    // Sessions
    //-------------------------------------------------------------------------
    enum class CreateSessionResult {
        Success,
        AccountUnavailable,
        DatabaseError
    };
    /**
     * Creates a session for an active account.
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
        enum class Result {
            Success,
            SessionNotFound,
            DatabaseError
        };

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

    enum class RevokeSessionResult {
        Success,
        SessionNotFound,
        DatabaseError
    };
    /**
     * Revokes the session with the given 32-byte token hash.
     */
    RevokeSessionResult revokeSession(const std::string& tokenHash);

    enum class RevokeAllSessionsResult {
        Success,
        DatabaseError
    };
    /**
     * Revokes every non-revoked session belonging to an account.
     */
    RevokeAllSessionsResult revokeAllSessions(Sint64 accountID);

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
     * @param targetServerID The specific server instance that may consume it.
     * @param expiresAt The Unix timestamp at which the ticket expires.
     */
    CreateServiceTicketResult createServiceTicket(
        Sint64 accountSessionID, const std::string& tokenHash,
        ServiceTicketAudience audience, Sint64 targetServerID,
        Sint64 expiresAt);

    struct ConsumedServiceTicketInfo {
        enum class Result {
            Success,
            TicketNotFound,
            DatabaseError
        };

        Result result{Result::DatabaseError};
        Sint64 accountSessionID{0};
        Sint64 accountID{0};
        std::string accountStatus{};
        Sint64 consumedAt{0};
    };
    /**
     * Atomically validates and consumes a service ticket.
     *
     * The ticket must be unconsumed, unrevoked, unexpired, and bound to the
     * given audience and target server. Its account session and account must
     * also remain valid.
     */
    ConsumedServiceTicketInfo consumeServiceTicket(
        const std::string& tokenHash, ServiceTicketAudience audience,
        Sint64 targetServerID);

protected:
    /**
     * Creates our tables in Accounts.db, if they don't already exist.
     */
    void initTables();

    /** File-backed database, storing account data. */
    SQLite::Database database;

    // Pre-built queries
    std::unique_ptr<SQLite::Statement> registerAccountQuery;
    std::unique_ptr<SQLite::Statement> insertRecoveryKeyQuery;
    std::unique_ptr<SQLite::Statement> getAccountLoginInfoQuery;
    std::unique_ptr<SQLite::Statement> createSessionQuery;
    std::unique_ptr<SQLite::Statement> validateSessionQuery;
    std::unique_ptr<SQLite::Statement> revokeSessionQuery;
    std::unique_ptr<SQLite::Statement> revokeAllSessionsQuery;
    std::unique_ptr<SQLite::Statement> createServiceTicketQuery;
    std::unique_ptr<SQLite::Statement> consumeServiceTicketQuery;
};

} // namespace AccountServer
} // namespace AM
