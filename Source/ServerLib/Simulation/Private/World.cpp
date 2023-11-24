#include "World.h"
#include "ClientSimData.h"
#include "ReplicatedComponentList.h"
#include "ReplicatedComponent.h"
#include "Position.h"
#include "PreviousPosition.h"
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
, itemData{}
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

entt::entity World::createEntity(const Position& position,
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

    // Add RelicatedComponentList so it gets updated as we add others.
    registry.emplace<ReplicatedComponentList>(newEntity);

    // All entities have a position.
    registry.emplace<Position>(newEntity, position);

    return newEntity;
}

void World::addGraphicsComponents(entt::entity entity,
                                  const AnimationState& animationState)
{
    // Note: We only add entities to the locator (and replicate them to clients)
    //       if they have an AnimationState. If we ever need to replicate 
    //       entities that don't have AnimationState, revisit this.
    //       Similarly, if we ever need to add AnimationState without Collision,
    //       we need to revisit this.

    // Add the AnimationState.
    registry.emplace<AnimationState>(entity, animationState);

    // TODO: When we add character sprite sets, update this.
    // Use the current sprite as the entity's collision bounds.
    const ObjectSpriteSet& spriteSet{
        spriteData.getObjectSpriteSet(animationState.spriteSetID)};
    const Sprite* sprite{spriteSet.sprites[animationState.spriteIndex]};

    const Position& position{registry.get<Position>(entity)};
    const Collision& collision{registry.emplace<Collision>(
        entity, sprite->modelBounds,
        Transforms::modelToWorldCentered(sprite->modelBounds, position))};

    // Add the entity to the locator.
    // Note: Since we're adding the entity to the locator, clients 
    //       will be told by ClientAOISystem to replicate it.
    entityLocator.setEntityLocation(entity, collision.worldBounds);
}

void World::addMovementComponents(entt::entity entity)
{
    registry.emplace<Input>(entity);
    // Note: All entities have a Position component.
    registry.emplace<PreviousPosition>(entity, registry.get<Position>(entity));
    // Note: We normally derive rotation from inputs, but we know the inputs 
    //       are default here.
    registry.emplace<Rotation>(entity);
}

std::string World::runInitScript(entt::entity entity,
                                 const InitScript& initScript)
{
    // If there's an init script, run it.
    // Note: We use "selfEntityID" to hold the ID of the entity that the init 
    //       script is being ran on.
    lua["selfEntityID"] = entity;
    auto result{lua.script(initScript.script, &sol::script_pass_on_error)};

    // If there was an error while running the init script, keep the entity 
    // alive (so the user can try again) and return the error.
    std::string returnString{""};
    std::string errorString{lua.get<std::string>("errorString")};
    if (!(result.valid())) {
        sol::error err = result;
        returnString = err.what();
    }
    else if (!(errorString.empty())) {
        returnString = errorString;
        lua["errorString"] = "";
    }
    else {
        // No errors, save the init script.
        registry.emplace<InitScript>(entity, initScript);
    }

    return returnString;
}

bool World::entityIDIsInUse(entt::entity entity) const
{
    // Note: We can't just check valid(), since calling create(N) will cause 
    //       valid() to return true for all X < N. We account for this by 
    //       checking if the index is actually in use.
    const auto* storage{registry.storage<entt::entity>()};
    if (registry.valid(entity)
        && (storage->index(entity) < storage->in_use())) {
        return true;
    }

    return false;
}

bool World::hasMovementComponents(entt::entity entity) const
{
    // Note: Entities also need Rotation and Collision for the movement systems
    //       to pick them up, but we assume those are present.
    return registry.all_of<Input, PreviousPosition>(entity);
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
