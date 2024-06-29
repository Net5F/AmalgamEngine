#include "World.h"
#include "EntityInitLua.h"
#include "ItemInitLua.h"
#include "Database.h"
#include "ClientSimData.h"
#include "ReplicatedComponentList.h"
#include "ReplicatedComponent.h"
#include "Position.h"
#include "PreviousPosition.h"
#include "Movement.h"
#include "GraphicState.h"
#include "Collision.h"
#include "EntityInitScript.h"
#include "PersistedEntityData.h"
#include "Deserialize.h"
#include "Transforms.h"
#include "SharedConfig.h"
#include "Config.h"
#include "VariantTools.h"
#include "StringTools.h"
#include "Log.h"
#include "AMAssert.h"
#include "sol/sol.hpp"
#include "boost/mp11/algorithm.hpp"
#include <type_traits>

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

World::World(GraphicData& inGraphicData, EntityInitLua& inEntityInitLua,
             ItemInitLua& inItemInitLua)
: registry{}
, itemData{}
, tileMap{inGraphicData}
, entityLocator{registry}
, entityStoredValueIDMap{}
, globalStoredValueMap{}
, database{std::make_unique<Database>()}
, netIDMap{}
, graphicData{inGraphicData}
, entityInitLua{inEntityInitLua}
, itemInitLua{inItemInitLua}
, nextStoredValueID{NULL_ENTITY_STORED_VALUE_ID + 1}
, workStringID{}
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
    entityLocator.setGridSize(tileMap.getTileExtent());

    // Add listeners for each client-relevant component. When the component is
    // constructed or destroyed, the associated entity's ReplicatedComponentList
    // will be updated.
    boost::mp11::mp_for_each<ReplicatedComponentTypes>([&](auto I) {
        using ComponentType = decltype(I);
        registry.on_construct<ComponentType>()
            .template connect<&onComponentConstructed<ComponentType>>();
        registry.on_destroy<ComponentType>()
            .template connect<&onComponentDestroyed<ComponentType>>();
    });

    // When an entity is destroyed, do any necessary cleanup.
    registry.on_destroy<entt::entity>().connect<&World::onEntityDestroyed>(
        this);

    // Load our saved non-client entities.
    loadNonClientEntities();

    // Load our saved item definitions.
    loadItems();

    // Load our saved stored value data.
    loadStoredValues();
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
                                  const GraphicState& graphicState)
{
    // Note: We only add entities to the locator (and replicate them to clients)
    //       if they have a GraphicState. If we ever need to replicate
    //       entities that don't have GraphicState, revisit this.
    //       Similarly, if we ever need to add GraphicState without Collision,
    //       we'll need to revisit this.

    // Add the GraphicState.
    registry.emplace<GraphicState>(entity, graphicState);

    // Use the current graphic as the entity's collision bounds.
    const EntityGraphicSet& graphicSet{
        graphicData.getEntityGraphicSet(graphicState.graphicSetID)};
    const GraphicRef& graphic{
        graphicSet.graphics.at(EntityGraphicType::IdleSouth)};

    const BoundingBox modelBounds{graphic.getModelBounds()};
    const Position& position{registry.get<Position>(entity)};
    const Collision& collision{registry.emplace<Collision>(
        entity, modelBounds,
        Transforms::modelToWorldCentered(modelBounds, position))};

    // Add the entity to the locator.
    // Note: Since we're adding the entity to the locator, clients
    //       will be told by ClientAOISystem to replicate it.
    entityLocator.setEntityLocation(entity, collision.worldBounds);
}

void World::addMovementComponents(entt::entity entity, const Rotation& rotation)
{
    if (!(registry.all_of<Input>(entity))) {
        registry.emplace<Input>(entity);
    }

    if (!(registry.all_of<PreviousPosition>(entity))) {
        // Note: All entities have a Position component.
        registry.emplace<PreviousPosition>(entity,
                                           registry.get<Position>(entity));
    }

    if (!(registry.all_of<Movement>(entity))) {
        registry.emplace<Movement>(entity);
    }

    if (!(registry.all_of<Rotation>(entity))) {
        // Note: We normally derive rotation from inputs, but we know the inputs
        //       are default here, and we may be constructing a persisted entity 
        //       that's facing a non-default direction.
        registry.emplace<Rotation>(entity, rotation);
    }

    // Note: Entities also need Collision for the movement systems to pick 
    //       them up, but that's considered a "graphics" component because it's 
    //       dependent on GraphicState.
}

std::string World::runEntityInitScript(entt::entity entity,
                                       const EntityInitScript& initScript)
{
    // Run the given script on the given entity.
    entityInitLua.selfEntity = entity;
    auto result{entityInitLua.luaState.script(initScript.script,
                                              &sol::script_pass_on_error)};

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

std::string World::runItemInitScript(Item& item,
                                     const ItemInitScript& initScript)
{
    // Run the given script on the given item.
    itemInitLua.selfItem = &item;
    auto result{itemInitLua.luaState.script(initScript.script,
                                            &sol::script_pass_on_error)};

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

EntityStoredValueID World::getEntityStoredValueID(std::string_view stringID)
{
    // Derive string ID in case the user accidentally passed a display name.
    StringTools::deriveStringID(stringID, workStringID);

    // If the value already exists, return its numeric ID.
    auto storedValueIDIt{entityStoredValueIDMap.find(workStringID)};
    if (storedValueIDIt != entityStoredValueIDMap.end()) {
        return storedValueIDIt->second;
    }
    else {
        // Check if we've ran out of IDs.
        if (nextStoredValueID == SDL_MAX_UINT16) {
            return NULL_ENTITY_STORED_VALUE_ID;
        }

        // Flag doesn't exist, add it to the map.
        EntityStoredValueID newFlagID{static_cast<Uint16>(nextStoredValueID)};
        entityStoredValueIDMap.emplace(workStringID, newFlagID);
        nextStoredValueID++;

        return newFlagID;
    }
}

void World::storeGlobalValue(std::string_view stringID, Uint32 newValue)
{
    // Derive string ID in case the user accidentally passed a display name.
    StringTools::deriveStringID(stringID, workStringID);

    // If we're setting the value to 0, don't add it to the map (default values 
    // don't need to be stored).
    if (newValue == 0) {
        // If the value already exists, erase it.
        auto valueIt{globalStoredValueMap.find(workStringID)};
        if (valueIt != globalStoredValueMap.end()) {
            globalStoredValueMap.erase(valueIt);
        }

        return;
    }

    globalStoredValueMap[workStringID] = newValue;
}

Uint32 World::getStoredValue(std::string_view stringID)
{
    // Derive string ID in case the user accidentally passed a display name.
    StringTools::deriveStringID(stringID, workStringID);

    // If the value exists, return it.
    auto valueIt{globalStoredValueMap.find(workStringID)};
    if (valueIt != globalStoredValueMap.end()) {
        return valueIt->second;
    }

    // Value doesn't exist. Return the default.
    return 0;
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

    // If the entity is in the database, delete it (does nothing if it isn't).
    database->deleteEntityData(entity);
}

void World::loadNonClientEntities()
{
    auto loadEntity
        = [&](entt::entity entity, const Uint8* entityDataBuffer,
              std::size_t dataSize) {
        // Deserialize the entity's data.
        PersistedEntityData persistedEntityData{};
        Deserialize::fromBuffer(entityDataBuffer, dataSize,
                                persistedEntityData);

        // Add the entity to the registry.
        entt::entity newEntity{registry.create(persistedEntityData.entity)};
        if (newEntity != persistedEntityData.entity) {
            LOG_FATAL("Created entity ID doesn't match saved entity ID. "
                      "Created: %u, saved: %u",
                      newEntity, persistedEntityData.entity);
        }

        // Add RelicatedComponentList so it gets updated as we add others.
        registry.emplace<ReplicatedComponentList>(newEntity);

        // Load the entity's persisted components into the registry.
        for (const PersistedComponent& componentVariant :
             persistedEntityData.components) {
            std::visit(VariantTools::Overload(
                [&](const Rotation& rotation) {
                    // Note: We only persist Rotation, but it implies the 
                    //       rest of the movement components.
                    addMovementComponents(newEntity, rotation);
                },
                [&](const GraphicState& graphicState) {
                    // Note: We only persist GraphicState, but it implies 
                    //       the rest of the graphics components.
                    addGraphicsComponents(newEntity, graphicState);
                },
                [&](const auto& component) {
                    using T = std::decay_t<decltype(component)>;
                    registry.emplace<T>(newEntity, component);
                }),
                componentVariant);
        }
    };

    database->iterateEntities(std::move(loadEntity));
}

void World::loadItems()
{
    auto loadItem = [&](ItemID itemID, const Uint8* itemDataBuffer,
                        std::size_t dataSize) {
        // Deserialize the item's data.
        Item item{};
        Deserialize::fromBuffer(itemDataBuffer, dataSize, item);

        // Add the item to ItemData.
        itemData.createItem(item);
    };

    database->iterateItems(std::move(loadItem));
}

void World::loadStoredValues()
{
    // Load the entity stored value IDs.
    auto loadEntityMap = [&](const Uint8* dataBuffer, std::size_t dataSize) {
        if (dataSize > 0) {
            Deserialize::fromBuffer(dataBuffer, dataSize,
                                    entityStoredValueIDMap);
        }
    };

    database->getEntityStoredValueIDMap(std::move(loadEntityMap));

    // Load the global stored values.
    auto loadGlobalMap = [&](const Uint8* dataBuffer, std::size_t dataSize) {
        if (dataSize > 0) {
            Deserialize::fromBuffer(dataBuffer, dataSize, globalStoredValueMap);
        }
    };

    database->getGlobalStoredValueMap(std::move(loadGlobalMap));
}

} // namespace Server
} // namespace AM
