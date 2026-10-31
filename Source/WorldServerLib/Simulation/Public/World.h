#pragma once

#include "TileMap.h"
#include "ConnectionHandle.h"
#include "EntityLocator.h"
#include "CollisionLocator.h"
#include "GlobalStoredValueMap.h"
#include "InventoryHelper.h"
#include "CastHelper.h"
#include "LoadHelper.h"
#include "SpawnStrategy.h"
#include "HashTools.h"
#include "entt/entity/registry.hpp"
#include <unordered_map>
#include <string_view>
#include <random>

namespace AM
{
class CastableData;
struct Position;
struct GraphicState;
struct EntityInitScript;
struct ItemInitScript;
struct Item;

namespace WorldServer
{
struct SimulationContext;
class Simulation;
class Network;
class GraphicData;
class ItemData;
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
    World(const SimulationContext& inSimContext);

    ~World();

    //-------------------------------------------------------------------------
    // World State
    //-------------------------------------------------------------------------
    /** Entity data registry. */
    entt::registry registry;

    /** Spatial partitioning grid for efficiently locating entities by
        their position. */
    EntityLocator entityLocator;

    /** Spatial partitioning grid for efficiently locating entities and tile
        layers by their collision volumes. */
    CollisionLocator collisionLocator;

    /** The tile map that makes up the world. */
    TileMap tileMap;

    /** Global key-value store.
        Note: "Global" means global to this world. Each world has its own map,
              and values aren't shared between worlds.
        Note: Stored values are often cast to different types, but their
              underlying type is always Uint32. */
    GlobalStoredValueMap globalStoredValueMap;

    /** Helper class for managing entity inventories. */
    InventoryHelper inventoryHelper;

    /** Helper class for casting Castables. */
    CastHelper castHelper;

    /** Helper class for loading persisted world state. */
    LoadHelper loadHelper;

    /** The database for saving and loading world data.
        Kept as a pointer to speed up compilation. */
    std::unique_ptr<Database> database;

    /** Maps client handles to entity IDs.
        Used for interfacing with the Network.
        Note: To go the other way (entt::entity -> ConnectionHandle), use
              ClientSimData. */
    std::unordered_map<ConnectionHandle, entt::entity> clientHandleMap;

    /** Maps stable IDs to the entity that holds them.
        Kept in sync with StableID components automatically.
        Note: To go the other way (entt::entity -> stable ID), use StableID.
        string_hash and equal_to are added for heterogeneous access. */
    std::unordered_map<std::string, entt::entity, string_hash, std::equal_to<>>
        stableIDMap;

    //-------------------------------------------------------------------------
    // Helper Functions
    //-------------------------------------------------------------------------
    /**
     * Returns the entity associated with the given client handle, or entt::null
     * if the client doesn't exist.
     *
     * Use this instead of directly using clientHandleMap.
     *
     * Note: To go the other way (entt::entity -> ConnectionHandle), use
     * ClientSimData.
     */
    entt::entity getClientEntity(ConnectionHandle clientHandle);

    /**
     * Teleports the given entity to the given position.
     * @return true if successful, else false. If false, the entity will
     *         remain at its original position.
     */
    bool teleportEntity(entt::entity entity, const Vector3& newPosition);

    /**
     * Creates an entity with the given position.
     *
     * @param entityHint (Optional) The entityID to use, if it's available.
     * @return The new entity's ID, or entt::null if a failure occurred
     *         (position outside of tile map bounds).
     */
    entt::entity createEntity(const Position& position,
                              entt::entity entityHint = entt::null);

    /**
     * Adds the given graphical component to the entity.
     *
     * Since an entity's collision is based on its graphics, this also adds
     * the Collision component and adds the entity to the locator.
     *
     * @return true if successful, else false (the graphic's collision + entity
     *         position went outside of the tile map bounds).
     */
    bool addGraphicsComponents(entt::entity entity,
                               const GraphicState& graphicState);

    /**
     * Adds the components needed for movement to the given entity.
     */
    void addMovementComponents(entt::entity entity);

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
    std::string runItemInitScript(Item& item, std::string_view initScript);

    /**
     * Adds a new value, or overwrites an existing value.
     *
     * If newValue == 0 (the default value), the value will be deleted.
     *
     * Note: Stored values are often cast to different types, but their
     *       underlying type is always Uint32.
     *
     * @param stringID The string ID of the value to add or overwrite.
     * @param newValue The new value to use.
     */
    void storeGlobalValue(std::string_view stringID, Uint32 newValue);

    /**
     * Gets a stored value.
     *
     * Note: Stored values are often cast to different types, but their
     *       underlying type is always Uint32.
     *
     * @param stringID The string ID of the value to get.
     * @return The requested value. If not found, returns 0 (the default value
     *         that the value would have if it existed).
     */
    Uint32 getStoredValue(std::string_view stringID);

    /**
     * Gives the entity the given stable ID, replacing any ID it already has.
     *
     * @param stableID The ID to set. Will be derived (see
     *                 StringTools::deriveStringID()).
     * @return true if successful, else false (ID is empty, too long, or
     *         already in use by a different entity).
     */
    bool setStableID(entt::entity entity, std::string_view stableID);

    /**
     * Returns the entity with the given stable ID, or entt::null if no entity
     * has it.
     *
     * Use this instead of directly using stableIDMap.
     *
     * @param stableID The ID to look up. Will be derived (see
     *                 StringTools::deriveStringID()).
     */
    entt::entity getEntityByStableID(std::string_view stableID);

    /**
     * Returns the spawn point position.
     * To configure, see WorldServer::Config.
     */
    Position getSpawnPoint();

    /**
     * Loads the saved world state from the database.
     * Should only be called by Simulation during construction.
     */
    void load();

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
     * Adds the entity's new stable ID to stableIDMap.
     */
    void onStableIDConstructed(entt::entity entity);

    /**
     * Removes the entity's stable ID from stableIDMap.
     */
    void onStableIDDestroyed(entt::entity entity);

    /** Used to get graphics info. */
    const GraphicData& graphicData;

    /** Used to run entity init scripts. */
    EntityInitLua& entityInitLua;

    /** Used to run item init scripts. */
    ItemInitLua& itemInitLua;

    /** A scratch buffer used while processing string IDs. */
    std::string workStringID;

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

} // namespace WorldServer
} // namespace AM
