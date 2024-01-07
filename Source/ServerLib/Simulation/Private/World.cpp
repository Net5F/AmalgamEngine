#include "World.h"
#include "Database.h"
#include "ClientSimData.h"
#include "ReplicatedComponentList.h"
#include "Input.h"
#include "Position.h"
#include "PreviousPosition.h"
#include "AnimationState.h"
#include "Collision.h"
#include "EntityInitScript.h"
#include "Transforms.h"
#include "SharedConfig.h"
#include "Config.h"
#include "Log.h"
#include "AMAssert.h"
#include "sol/sol.hpp"
#include "boost/mp11/algorithm.hpp"

namespace AM
{
namespace Server
{

World::World(SpriteData& inSpriteData, sol::state& inEntityInitLua,
             sol::state& inItemInitLua)
: registry{}
, itemData{}
, tileMap{inSpriteData}
, entityLocator{registry}
, database{std::make_unique<Database>()}
, netIdMap{}
, spriteData{inSpriteData}
, entityInitLua{inEntityInitLua}
, itemInitLua{inItemInitLua}
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

    // Load our saved non-client entities.
    loadNonClientEntities();

    // Load our saved item definitions.
    loadItems();
}

World::~World() = default;

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

std::string World::runEntityInitScript(entt::entity entity,
                                       const EntityInitScript& initScript)
{
    // Run the given script on the given entity.
    // Note: We use "selfEntityID" to hold the ID of the entity that the init
    //       script is being ran on.
    entityInitLua["selfEntityID"] = entity;
    auto result{
        entityInitLua.script(initScript.script, &sol::script_pass_on_error)};

    // If the init script ran successfully, save it.
    std::string returnString{""};
    if (result.valid()) {
        registry.emplace<EntityInitScript>(entity, initScript);
    }
    else {
        // Error while running the init script. Keep the entity alive (so the
        // user can try again) and return the error.
        sol::error err = result;
        returnString = err.what();
    }

    return returnString;
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

std::string World::runItemInitScript(Item& item,
                                     const ItemInitScript& initScript)
{
    // Run the given script on the given item.
    // Note: We use "selfItemPtr" to hold the item that is being initialized.
    itemInitLua["selfItemPtr"] = &item;
    auto result{
        itemInitLua.script(initScript.script, &sol::script_pass_on_error)};

    // If the init script ran successfully, save it.
    std::string returnString{""};
    if (result.valid()) {
        item.initScript = initScript;
    }
    else {
        // Error while running the init script. Return the error.
        sol::error err = result;
        returnString = err.what();
    }

    return returnString;
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

void World::loadNonClientEntities()
{
    //auto loadEntity
    //    = [&](entt::entity entity, std::string_view serializedComponents) {
    //};

    //database->iterateEntityData(loadEntity);
}

void World::loadItems()
{
}

} // namespace Server
} // namespace AM
