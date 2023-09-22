#include "World.h"
#include "ClientSimData.h"
#include "ReplicatedComponentList.h"
#include "ReplicatedComponent.h"
#include "Position.h"
#include "AnimationState.h"
#include "Collision.h"
#include "InitScript.h"
#include "Transforms.h"
#include "SharedConfig.h"
#include "Config.h"
#include "Log.h"
#include "Ignore.h"
#include "AMAssert.h"
#include "sol/sol.hpp"
#include "boost/mp11/algorithm.hpp"

namespace AM
{
namespace Server
{

template<typename T>
void onComponentConstructed(entt::registry& registry, entt::entity entity)
{
    // Find the component's index within the type list.
    constexpr std::size_t index{
        boost::mp11::mp_find<ReplicatedComponentTypes, T>::value};

    // Add the component to the entity's tracking vector.
    auto& replicatedComponents{
        registry.get_or_emplace<ReplicatedComponentList>(entity)};
    replicatedComponents.typeIndices.push_back(static_cast<Uint8>(index));
}

template<typename T>
void onComponentDestroyed(entt::registry& registry, entt::entity entity)
{
    // Find the component's index within the type list.
    constexpr std::size_t index{
        boost::mp11::mp_find<ReplicatedComponentTypes, T>::value};

    // If the component is in the entity's tracking vector, remove it.
    if (auto replicatedComponents
        = registry.try_get<ReplicatedComponentList>(entity)) {
        std::erase(replicatedComponents->typeIndices, index);
    }
}

World::World(SpriteData& inSpriteData, sol::state& inLua)
: registry{}
, tileMap{inSpriteData}
, entityLocator{registry}
, netIdMap{}
, spriteData{inSpriteData}
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

    // Add listeners for each client-relevant component. When the component is 
    // constructed or destroyed, the associated entity's ReplicatedComponentList
    // will be updated.
    boost::mp11::mp_for_each<ReplicatedComponentTypes>([&](auto I) {
        using T = decltype(I);
        registry.on_construct<T>()
            .template connect<&onComponentConstructed<T>>();
        registry.on_destroy<T>().template connect<&onComponentDestroyed<T>>();
    });

    // When an entity is destroyed, do any necessary cleanup.
    registry.on_destroy<entt::entity>().connect<&World::onEntityDestroyed>(
        this);
}

entt::entity World::constructEntity(std::span<const ReplicatedComponent> components,
                                    const InitScript& initScript,
                                    entt::entity entityHint)
{
    // Create the new entity.
    entt::entity newEntity{entt::null};
    if (entityHint != entt::null) {
        newEntity = registry.create(entityHint);
    }
    else {
        newEntity = registry.create();
    }

    // Add RelicatedComponentList first so it gets updated as we add others.
    registry.emplace<ReplicatedComponentList>(newEntity);

    // Add the given components.
    for (const auto& componentVariant : components) {
        std::visit([&](const auto& component) {
            using T = std::decay_t<decltype(component)>;
            registry.emplace<T>(newEntity, component);
        }, componentVariant);
    }

    // Add any additional components based on those that were added.
    // Note: Be careful with holding onto references here. If components 
    //       are added to the same group, the ref will be invalidated.

    // Note: We only add entities to the locator (and replicate them to clients)
    //       if they have both Position and AnimationState. If we ever need 
    //       to replicate entities that don't have AnimationState, revisit this.
    // If the entity has a position and animation state, add collision and 
    // add the entity to the locator.
    const auto [position, animationState]
        = registry.try_get<Position, AnimationState>(newEntity);
    if (position && animationState) {
        const ObjectSpriteSet& spriteSet{
            spriteData.getObjectSpriteSet(animationState->spriteSetID)};
        const Sprite* sprite{spriteSet.sprites[animationState->spriteIndex]};

        const Collision& collision{registry.emplace<Collision>(
            newEntity, sprite->modelBounds,
            Transforms::modelToWorldCentered(sprite->modelBounds, *position))};

        // Note: Since the entity was added to the locator, clients 
        //       will be told by ClientAOISystem to replicate it.
        entityLocator.setEntityLocation(newEntity, collision.worldBounds);
    }

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
