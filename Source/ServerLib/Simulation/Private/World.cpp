#include "World.h"
#include "ClientSimData.h"
#include "EntityType.h"
#include "Name.h"
#include "Position.h"
#include "Collision.h"
#include "InitScript.h"
#include "Transforms.h"
#include "SharedConfig.h"
#include "Config.h"
#include "Log.h"
#include "Ignore.h"
#include "sol/sol.hpp"

namespace AM
{
namespace Server
{
World::World(SpriteData& spriteData, sol::state& inLua)
: registry{}
, tileMap{spriteData}
, entityLocator{registry}
, netIdMap{}
, lua{inLua}
, randomDevice{}
, generator{randomDevice()}
, xDistribution{Config::SPAWN_POINT_RANDOM_MIN_X,
                Config::SPAWN_POINT_RANDOM_MAX_X}
, yDistribution{Config::SPAWN_POINT_RANDOM_MIN_Y,
                Config::SPAWN_POINT_RANDOM_MAX_Y}
, groupX{Config::SPAWN_POINT_GROUP_MIN_X}
, groupY{Config::SPAWN_POINT_GROUP_MIN_Y}
, columnIndex{0}
, rowIndex{0}
{
    // Allocate the entity locator's grid.
    entityLocator.setGridSize(tileMap.getTileExtent().xLength,
                              tileMap.getTileExtent().yLength);

    // When an entity is destroyed, do any necessary cleanup.
    registry.on_destroy<entt::entity>().connect<&World::onEntityDestroyed>(
        this);
}

entt::entity World::constructDynamicObject(const Name& name,
                                           const Position& position,
                                           const Rotation& rotation,
                                           const ObjectSpriteSet& spriteSet,
                                           const InitScript& initScript,
                                           entt::entity entityHint)
{
    // Create the new entity and initialize it.
    entt::entity newEntity{entt::null};
    if (entityHint != entt::null) {
        newEntity = registry.create(entityHint);
    }
    else {
        newEntity = registry.create();
    }

    // Construct the standard components.
    // Note: Be careful with holding onto references here. If components 
    //       are added to the same group, the ref will be invalidated.
    registry.emplace<EntityType>(newEntity, EntityType::DynamicObject);
    registry.emplace<Name>(newEntity, name);

    registry.emplace<Position>(newEntity, position);
    registry.emplace<Rotation>(newEntity, rotation);

    registry.emplace<ObjectSpriteSet>(newEntity, spriteSet);

    // Note: The server doesn't have any need for a Sprite component on dynamic
    //       entities, we just get it from the sprite set + rotation.
    const Sprite& sprite{
        *(spriteSet.sprites[registry.get<Rotation>(newEntity).direction])};

    // Note: Every entity needs a Collision for the EntityLocator to use.
    const Collision& collision{registry.emplace<Collision>(
        newEntity, sprite.modelBounds,
        Transforms::modelToWorldCentered(sprite.modelBounds,
                                         registry.get<Position>(newEntity)))};

    // Start tracking the entity in the locator.
    // Note: Since the entity was added to the locator, clients 
    //       will be told by ClientAOISystem to construct it.
    entityLocator.setEntityLocation(newEntity, collision.worldBounds);

    registry.emplace<InitScript>(newEntity, initScript);

    // Run the given init script.
    lua["selfEntityID"] = newEntity;
    lua.script(initScript.script, &sol::script_default_on_error);

    return newEntity;
}

bool World::entityIDIsInUse(entt::entity entityID)
{
    // Note: We can't just check valid(), since calling create(N) will cause 
    //       valid() to return true for all X < N. We account for this by 
    //       checking if the index is actually in use.
    const auto& storage{registry.storage<entt::entity>()};
    if (registry.valid(entityID)
        && (storage.index(entityID) < storage.in_use())) {
        return true;
    }

    return false;
}

Position World::getSpawnPoint()
{
    switch (Config::SPAWN_STRATEGY) {
        case SpawnStrategy::Fixed: {
            return {Config::SPAWN_POINT_FIXED_X, Config::SPAWN_POINT_FIXED_Y};
        }
        case SpawnStrategy::Random: {
            return {xDistribution(generator), yDistribution(generator), 0};
        }
        case SpawnStrategy::Grouped: {
            return getGroupedSpawnPoint();
        }
        default: {
            LOG_FATAL("Invalid spawn strategy.");
            return {};
        }
    }
}

Position World::getGroupedSpawnPoint()
{
    // Calculate the next spawn point.
    Position spawnPoint{groupX, groupY};
    spawnPoint.x += (columnIndex * Config::SPAWN_POINT_GROUP_PADDING_X);
    spawnPoint.y += (rowIndex * Config::SPAWN_POINT_GROUP_PADDING_Y);

    // Increment our column. If it wrapped, increment our row.
    columnIndex = ((columnIndex + 1) % Config::SPAWN_POINT_GROUP_COLUMNS);
    unsigned int previousRow{rowIndex};
    if (columnIndex == 0) {
        rowIndex = ((rowIndex + 1) % Config::SPAWN_POINT_GROUP_ROWS);
    }

    // If the row wrapped, increment our group position.
    if (previousRow > rowIndex) {
        groupX += Config::SPAWN_POINT_GROUP_OFFSET_X;
        groupY += Config::SPAWN_POINT_GROUP_OFFSET_Y;
    }

    return spawnPoint;
}

void World::onEntityDestroyed(entt::entity entity)
{
    // Remove it from the locator.
    entityLocator.removeEntity(entity);
}

} // namespace Server
} // namespace AM
