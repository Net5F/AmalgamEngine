#pragma once

#include "SQLiteCpp/SQLiteCpp.h"
#include <memory>
#include <string_view>

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
};

} // namespace AccountServer
} // namespace AM
