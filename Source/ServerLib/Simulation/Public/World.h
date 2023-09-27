#pragma once

#include "TileMap.h"
#include "NetworkDefs.h"
#include "Position.h"
#include "ReplicatedComponent.h"
#include "EntityLocator.h"
#include "SpawnStrategy.h"
#include "entt/entity/registry.hpp"
#include <unordered_map>
#include <random>
#include <span>

namespace sol
{
class state;
}

namespace AM
{
struct Name;
struct Position;
struct AnimationState;

namespace Server
{
class SpriteData;
struct InitScript;

/**
 * Owns and manages the persistence of all world state.
 *
 * The server's world state consists of:
 *   Map data
 *     See TileMap.h
 *   Entity data
 *     Maintained at runtime in an ECS registry.
 *     Eventually will be persisted in a database.
 *
 * Also provides helpers for common uses of world state.
 */
class World
{
public:
    World(SpriteData& inSpriteData, sol::state& inLua);

    /** Entity data registry. */
    entt::registry registry;

    /** The tile map that makes up the world. */
    TileMap tileMap;

    /** Spatial partitioning grid for efficiently locating entities by
        position. */
    EntityLocator entityLocator;

    /** Maps network IDs to entity IDs.
        Used for interfacing with the Network. */
    std::unordered_map<NetworkID, entt::entity> netIdMap;

    /**
     * Constructs an entity with the given components, then runs initScript 
     * on it.
     * 
     * Note: We restrict the component types to those in ReplicatedComponent
     *       only because it has all that we currently need and it's convenient.
     */
    entt::entity
        constructEntity(const Position& position,
                        std::span<const ReplicatedComponent> components,
                        const InitScript& initScript,
                        entt::entity entityHint = entt::null);

    /**
     * Returns true if the given ID is valid and in use.
     * Note: If entt adds a storage.in_use(entity), we can replace this.
     */
    bool entityIDIsInUse(entt::entity entityID);

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
    sol::state& lua;

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
