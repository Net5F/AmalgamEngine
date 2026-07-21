#pragma once

#include "SQLiteCpp/SQLiteCpp.h"
#include <optional>
#include <memory>

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
     * queued until commitTransaction() is called.
     */
    void startTransaction();

    /**
     * Overload to use a non-default behavior.
     */
    void startTransaction(SQLite::TransactionBehavior behavior);

    /**
     * If a transaction is ongoing, commits it. This will execute all queued
     * queries.
     */
    void commitTransaction();

    //-------------------------------------------------------------------------
    // Accounts
    //-------------------------------------------------------------------------

protected:
    /**
     * Creates our tables in Accounts.db, if they don't already exist.
     */
    void initTables();

    /** File-backed database, storing account data. */
    SQLite::Database database;

    /** If valid, this is the current ongoing transaction. */
    std::optional<SQLite::Transaction> currentTransaction;

    // Pre-built queries
    std::unique_ptr<SQLite::Statement> insertEntityQuery;
};

} // namespace AccountServer
} // namespace AM
