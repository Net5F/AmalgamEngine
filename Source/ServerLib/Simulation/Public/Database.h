#pragma once

#include "SQLiteCpp/SQLiteCpp.h"
#include <memory>
#include <functional>
#include <string_view>

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
     * Calls the given callback on each entity data entry.
     *
     * @param callback A callback of form void(entt::entity, std::string_view) 
     *                 that expects the entity's ID, and a serialized
     *                 std::vector<ReplicatedComponent>.
     */
    //template<typename Func>
    //void iterateEntityData(Func callback)
    //{
    //    SQLite::Statement query{database, "SELECT * FROM entities"};
    //    while (query.executeStep()) {
    //        callback(query.getColumn(0), query.getColumn(1));
    //    }
    //}

protected:
    /**
     * Creates our tables, if they don't already exist.
     *
     * Note: If things ever get sufficiently complicated, we can switch to a 
     *       schema.
     */
    void initTables();

    SQLite::Database database;
};

} // namespace Server
} // namespace AM
