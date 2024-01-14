#pragma once

#include "SQLiteCpp/SQLiteCpp.h"
#include "entt/fwd.hpp"
#include <SDL_stdinc.h>
#include <optional>
#include <memory>

namespace AM
{
namespace Server
{
class World;

/**
 * Interface for interacting with the database.
 * 
 * We use the database to persist item definitions and non-client entity data 
 * as blobs.
 *
 * Note: Client entity data is persisted in the account database, not here.
 * TODO: Should we persist the tile map inside of the DB instead of a separate 
 *       file? We could track which chunks are modified and only save those.
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
     * If a transaction is ongoing, commits it. This will execute all queued 
     * queries.
     */
    void commitTransaction();

    /**
     * Adds or overwrites an entity table entry.
     *
     * @param entity The entity entry to update.
     * @param entityDataBuffer A serialized PersistedEntityData struct.
     * @param dataSize The size of persistedEntityData.
     * Note: We would normally use a std::span, but it would be extra work 
     *       in this case for no gain.
     */
    void saveEntityData(entt::entity entity, Uint8* entityDataBuffer,
                        std::size_t dataSize);

    /**
     * Attempts to delete an entity table entry for the given entity.
     *
     * If the entity is not found in the database, does nothing.
     */
    void deleteEntityData(entt::entity entity);

    /**
     * Calls the given callback on each entity data entry.
     *
     * @param callback A callback of form void(entt::entity, std::string_view) 
     *                 that expects the entity's ID, and a serialized
     *                 std::vector<ReplicatedComponent>.
     */
    template<typename Func>
    void iterateEntities(Func callback)
    {
        while (iterateEntitiesQuery->executeStep()) {
            SQLite::Column dataColumn{iterateEntitiesQuery->getColumn(1)};
            callback(static_cast<entt::entity>(
                         iterateEntitiesQuery->getColumn(0).getInt()),
                     static_cast<const Uint8*>(dataColumn.getBlob()),
                     dataColumn.getBytes());
        }
    }

protected:
    /**
     * Creates our tables, if they don't already exist.
     *
     * Note: If things ever get sufficiently complicated, we can switch to a 
     *       schema.
     */
    void initTables();

    SQLite::Database database;

    /** If valid, this is a current ongoing transaction. */
    std::optional<SQLite::Transaction> currentTransaction;

    // Pre-built queries
    std::unique_ptr<SQLite::Statement> insertEntityQuery;
    std::unique_ptr<SQLite::Statement> deleteEntityQuery;
    std::unique_ptr<SQLite::Statement> iterateEntitiesQuery;
};

} // namespace Server
} // namespace AM
