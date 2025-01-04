#include "World.h"
#include "GraphicData.h"
#include "EnttGroups.h"
#include "EntityInitLua.h"
#include "ItemInitLua.h"
#include "Database.h"
#include "ClientSimData.h"
#include "ReplicatedComponentList.h"
#include "ReplicatedComponent.h"
#include "EnginePersistedComponentTypes.h"
#include "ProjectPersistedComponentTypes.h"
#include "Position.h"
#include "PreviousPosition.h"
#include "Movement.h"
#include "MovementModifiers.h"
#include "GraphicState.h"
#include "Collision.h"
#include "EntityInitScript.h"
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
, entityLocator{registry}
, collisionLocator{}
, tileMap{inGraphicData, collisionLocator}
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
, groupX{}
, groupY{}
, columnIndex{0}
, rowIndex{0}
{
    // Initialize our entt groups.
    EnttGroups::init(registry);

    // Calc our group spawn point starting position. We add padding to make 
    // sure they don't clip the North or West edges of the map.
    TileExtent tileMapExtent{tileMap.getTileExtent()};
    groupX
        = tileMapExtent.x * static_cast<float>(SharedConfig::TILE_WORLD_WIDTH)
          + Config::SPAWN_POINT_GROUP_PADDING_X;
    groupY
        = tileMapExtent.y * static_cast<float>(SharedConfig::TILE_WORLD_WIDTH)
          + Config::SPAWN_POINT_GROUP_PADDING_Y;

    // Allocate the entity locator grid.
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

bool World::teleportEntity(entt::entity entity, const Vector3& newPosition)
{
    auto movementGroup{EnttGroups::getMovementGroup(registry)};

    auto [position, collision] = movementGroup.get<Position, Collision>(entity);
    position = newPosition;
    collision.worldBounds
        = Transforms::modelToWorldEntity(collision.modelBounds, position);

    // Flag that the entity's movement state needs to be synced.
    // (movement state is auto-synced when Input is dirtied).
    registry.patch<Input>(entity, [](auto&) {});

    // TODO: Check if valid and return false if not.
    return true;
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
    entityLocator.updateEntity(newEntity, position);

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
        Transforms::modelToWorldEntity(modelBounds, position))};

    // Entities with Collision get added to the locator.
    CollisionObjectType::Value objectType{
        registry.all_of<IsClientEntity>(entity)
            ? CollisionObjectType::ClientEntity
            : CollisionObjectType::NonClientEntity};
    collisionLocator.updateEntity(entity, collision.worldBounds, objectType);
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

    if (!(registry.all_of<MovementModifiers>(entity))) {
        registry.emplace<MovementModifiers>(entity);
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
            return {Config::SPAWN_POINT_FIXED_X, Config::SPAWN_POINT_FIXED_Y,
                    0.1f};
        }
        case SpawnStrategy::Random: {
            return {xDistribution(generator), yDistribution(generator), 0.1f};
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
    static constexpr float TILE_WIDTH{SharedConfig::TILE_WORLD_WIDTH};

    // Calculate the next spawn point.
    Position spawnPoint{groupX, groupY, 0.1f};
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
        // The width of a full group of entities. We add one extra padding to 
        // make sure they don't clip the Eastern edge of the map.
        const float GROUP_WIDTH{Config::SPAWN_POINT_GROUP_PADDING_X
                                    * Config::SPAWN_POINT_GROUP_COLUMNS
                                + Config::SPAWN_POINT_GROUP_PADDING_X};

        // Increment the group X offset.
        groupX += Config::SPAWN_POINT_GROUP_OFFSET_X;

        // If the new group would go off the East edge of the map, reset the 
        // X offset and increment the Y offset.
        TileExtent tileMapExtent{tileMap.getTileExtent()};
        float tileMapMaxX{tileMapExtent.xMax() * TILE_WIDTH};
        if ((groupX + GROUP_WIDTH) > tileMapMaxX) {
            groupX = tileMapExtent.x * TILE_WIDTH
                     + Config::SPAWN_POINT_GROUP_PADDING_X;
            groupY += Config::SPAWN_POINT_GROUP_OFFSET_Y;
        }

        columnIndex = 0;
        rowIndex = 0;
    }

    return spawnPoint;
}

void World::onEntityDestroyed(entt::entity entity)
{
    // Remove it from the locators.
    // Note: Client entities could easily be removed where we delete them, but 
    //       NCEs may be deleted at any point by project code, so we handle it
    //       here to avoid bugs. 
    entityLocator.removeEntity(entity);
    if (registry.all_of<Collision>(entity)) {
        collisionLocator.removeEntity(entity);
    }

    // If the entity is in the database, delete it (does nothing if it isn't).
    // Note: This is to delete non-client entities, since they get persisted.
    database->deleteEntityData(entity);
}

void World::loadNonClientEntities()
{
    std::vector<EnginePersistedComponent> engineComponents{};
    std::vector<ProjectPersistedComponent> projectComponents{};
    auto loadEntity = [&](entt::entity entity,
                          std::span<const Uint8> engineComponentBuffer,
                          std::span<const Uint8> projectComponentBuffer) {
        engineComponents.clear();
        projectComponents.clear();

        // Deserialize the entity's component data.
        Deserialize::fromBuffer(engineComponentBuffer.data(),
                                engineComponentBuffer.size(), engineComponents);
        Deserialize::fromBuffer(projectComponentBuffer.data(),
                                projectComponentBuffer.size(),
                                projectComponents);

        // Find the Position component.
        // Note: We do this separately because we know every entity has a 
        //       Position, and we need it for createEntity() (and we want to 
        //       use createEntity() to centralize logic and avoid bugs).
        const Position* position{nullptr};
        for (const EnginePersistedComponent& componentVariant :
             engineComponents) {
            if (const Position*
                tempPosition{std::get_if<Position>(&componentVariant)}) {
                position = tempPosition;
                break;
            }
        }
        if (!position) {
            LOG_INFO("Tried to load entity with no Position. Skipping.");
            return;
        }

        // Add the entity to the registry.
        entt::entity newEntity{createEntity(*position, entity)};
        if (newEntity != entity) {
            LOG_FATAL("Created entity ID doesn't match saved entity ID. "
                      "Created: %u, saved: %u",
                      newEntity, entity);
        }

        // Load the entity's persisted components into the registry.
        // Engine components
        for (const EnginePersistedComponent& componentVariant :
             engineComponents) {
            std::visit(VariantTools::Overload(
                [&](const Position&) {
                    // Do nothing, we already added the position above.
                },
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

        // Project components
        for (const ProjectPersistedComponent& componentVariant :
             projectComponents) {
            std::visit(VariantTools::Overload(
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
    auto loadItem = [&](ItemID itemID, std::span<const Uint8> itemBuffer) {
        // Deserialize the item's data.
        Item item{};
        Deserialize::fromBuffer(itemBuffer.data(), itemBuffer.size(), item);
        item.numericID = itemID;

        // Add the item to ItemData.
        itemData.createItem(item);
    };

    database->iterateItems(std::move(loadItem));
}

void World::loadStoredValues()
{
    // Load the entity stored value IDs.
    auto loadEntityMap = [&](std::span<const Uint8> dataBuffer) {
        if (dataBuffer.size() > 0) {
            Deserialize::fromBuffer(dataBuffer.data(), dataBuffer.size(),
                                    entityStoredValueIDMap);
        }
    };

    database->getEntityStoredValueIDMap(std::move(loadEntityMap));

    // Load the global stored values.
    auto loadGlobalMap = [&](std::span<const Uint8> dataBuffer) {
        if (dataBuffer.size() > 0) {
            Deserialize::fromBuffer(dataBuffer.data(), dataBuffer.size(),
                                    globalStoredValueMap);
        }
    };

    database->getGlobalStoredValueMap(std::move(loadGlobalMap));
}

} // namespace Server
} // namespace AM
