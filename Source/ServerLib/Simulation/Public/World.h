#pragma once

#include "ItemData.h"
#include "TileMap.h"
#include "NetworkDefs.h"
#include "EntityLocator.h"
#include "SpawnStrategy.h"
#include "entt/entity/registry.hpp"
#include <unordered_map>
#include <random>

namespace sol
{
class state;
}

namespace AM
{
struct Name;
struct Position;
struct AnimationState;
struct EntityInitScript;
struct ItemInitScript;

namespace Server
{
class SpriteData;

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
    World(SpriteData& inSpriteData, sol::state& inEntityInitLua,
          sol::state& inItemInitLua);

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
                               const AnimationState& animationState);

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
     * Returns true if the given ID is valid and in use.
     * Note: If entt adds a storage.in_use(entity), we can replace this.
     */
    bool entityIDIsInUse(entt::entity entity) const;

    /**
     * Returns true if the given entity has all the components necessary for 
     * movement.
     */
    bool hasMovementComponents(entt::entity entity) const;

    /**
     * Runs the given init script on the given item. If successful, saves it  
     * in item.initScript.
     *
     * @return If the init script failed to run, returns a string 
     *         describing the issue. Else, returns an empty string.
     */
    std::string runItemInitScript(Item& item,
                                  const ItemInitScript& initScript);

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

    /** Used to get sprite info. */
    const SpriteData& spriteData;

    /** Used to run entity init scripts. */
    sol::state& entityInitLua;

    /** Used to run item init scripts. */
    sol::state& itemInitLua;

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
