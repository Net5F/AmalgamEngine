#pragma once

#include "ItemData.h"
#include "TileMap.h"
#include "NetworkDefs.h"
#include "EntityLocator.h"
#include "SpawnStrategy.h"
#include "entt/entity/registry.hpp"
#include <unordered_map>
#include <random>

namespace AM
{
struct Position;
struct GraphicState;
struct EntityInitScript;
struct ItemInitScript;

namespace Server
{
class GraphicData;
class Database;
struct EntityInitLua;
struct ItemInitLua;

/**
 * Owns and manages the persistence of all world state.
 *
 * The server's world state consists of:
 *   Map data
 *     See TileMap.h
 *   Entity data
 *     Maintained at runtime in an ECS registry.
 *     Eventually will be persisted in a database.
 *   Item data
 *     Stored as "item templates", modifiable at runtime.
 *     Eventually will be persisted in a database.
 *
 * Also provides helpers for common uses of world state.
 */
class World
{
public:
    World(GraphicData& inGraphicData, EntityInitLua& inEntityInitLua,
          ItemInitLua& inItemInitLua);

    ~World();

    //-------------------------------------------------------------------------
    // World State
    //-------------------------------------------------------------------------
    /** Entity data registry. */
    entt::registry registry;

    /** Item data templates. */
    ItemData itemData;

    /** The tile map that makes up the world. */
    TileMap tileMap;

    /** Spatial partitioning grid for efficiently locating entities by
        position. */
    EntityLocator entityLocator;

    /** The database for saving and loading world data.
        Kept as a pointer to speed up compilation. */
    std::unique_ptr<Database> database;

    /** Maps network IDs to entity IDs.
        Used for interfacing with the Network. */
    std::unordered_map<NetworkID, entt::entity> netIdMap;

    //-------------------------------------------------------------------------
    // Helper Functions
    //-------------------------------------------------------------------------
    /**
     * Creates an entity with the given position.
     *
     * @param entityHint (Optional) The entityID to use, if it's available.
     * @return The new entity's ID.
     */
    entt::entity createEntity(const Position& position,
                              entt::entity entityHint = entt::null);

    /**
     * Adds the given graphical component to the entity.
     *
     * Since an entity's collision is based on its graphics, this also adds
     * the Collision component and adds the entity to the locator.
     */
    void addGraphicsComponents(entt::entity entity,
                               const GraphicState& graphicState);

    /**
     * Adds the components needed for movement to the given entity.
     * @param rotation The rotation to apply. Only used if the entity doesn't 
     *                 already have a Rotation component.
     */
    void addMovementComponents(entt::entity entity, const Rotation& rotation);

    /**
     * Runs the given init script on the given entity. If successful, adds it
     * as an EntityInitScript component.
     *
     * @return If the init script failed to run, returns a string
     *         describing the issue. Else, returns an empty string.
     */
    std::string runEntityInitScript(entt::entity entity,
                                    const EntityInitScript& initScript);

    /**
     * Runs the given init script on the given item. If successful, saves it
     * in item.initScript.
     *
     * @return If the init script failed to run, returns a string
     *         describing the issue. Else, returns an empty string.
     */
    std::string runItemInitScript(Item& item, const ItemInitScript& initScript);

    /**
     * Returns the spawn point position.
     * To configure, see Server::Config.
     */
    Position getSpawnPoint();

private:
    /**
     * Returns the next spawn point, trying to build groups of 10.
     */
    Position getGroupedSpawnPoint();

    /**
     * Does any necessary cleanup to the given entity.
     */
    void onEntityDestroyed(entt::entity entity);

    /**
     * Loads our saved non-client entities and adds them to the registry.
     */
    void loadNonClientEntities();

    /**
     * Loads our saved items and adds them to itemData.
     */
    void loadItems();

    /** Used to get graphics info. */
    const GraphicData& graphicData;

    /** Used to run entity init scripts. */
    EntityInitLua& entityInitLua;

    /** Used to run item init scripts. */
    ItemInitLua& itemInitLua;

    // For random spawn points.
    std::random_device randomDevice;
    std::mt19937 generator;
    std::uniform_real_distribution<float> xDistribution;
    std::uniform_real_distribution<float> yDistribution;

    // For grouped spawn points.
    float groupX;
    float groupY;
    unsigned int columnIndex;
    unsigned int rowIndex;
};

} // namespace Server
} // namespace AM
