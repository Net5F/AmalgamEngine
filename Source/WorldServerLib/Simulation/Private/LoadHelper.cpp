#include "LoadHelper.h"
#include "World.h"
#include "Simulation.h"
#include "Database.h"
#include "ItemData.h"
#include "EnginePersistedComponentTypes.h"
#include "ProjectPersistedComponentTypes.h"
#include "ComponentMigration.h"
#include "EngineComponentMigrationFunctions.h"
#include "ComponentMigrationFunctions.h"
#include "Position.h"
#include "Input.h"
#include "Rotation.h"
#include "GraphicState.h"
#include "Collision.h"
#include "CollisionBitSets.h"
#include "CastCooldown.h"
#include "SaveTimestamp.h"
#include "PersistedComponentDefs.h"
#include "GraphicData.h"
#include "SharedConfig.h"
#include "Transforms.h"
#include "Deserialize.h"
#include "ByteTools.h"
#include "Log.h"
#include "entt/core/type_info.hpp"
#include "boost/mp11/algorithm.hpp"
#include <algorithm>
#include <functional>
#include <optional>
#include <span>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace AM
{
namespace WorldServer
{

LoadHelper::LoadHelper(World& inWorld, Simulation& inSimulation,
                       ItemData& inItemData, const GraphicData& inGraphicData)
: world{inWorld}
, simulation{inSimulation}
, itemData{inItemData}
, graphicData{inGraphicData}
{
}

void LoadHelper::loadNonClientEntities()
{
    // Iterate and load each entity in the database (we only store non-client 
    // entities in the database).
    world.database->iterateEntities(
        std::bind_front(&LoadHelper::loadEntity, this));
}

void LoadHelper::loadItems()
{
    auto loadItem = [&](ItemID itemID, std::span<const Uint8> serializedItem,
                        ItemVersion version, std::string_view initScript) {
        // Initialize the item's non-script-provided fields.
        Item item{.numericID = itemID};
        if (!Deserialize::fromBuffer(serializedItem.data(),
                                     serializedItem.size(), item)) {
            LOG_ERROR("Item failed to deserialize: %u", itemID);
            return;
        }

        // Add the item to ItemData.
        itemData.loadItem(item, version, initScript);
    };

    world.database->iterateItems(std::move(loadItem));
}

void LoadHelper::loadGlobalStoredValues()
{
    auto loadGlobalMap = [&](std::span<const Uint8> serializedMap) {
        if (serializedMap.empty()) {
            return;
        }

        if (!Deserialize::fromBuffer(serializedMap.data(), serializedMap.size(),
                                     world.globalStoredValueMap)) {
            LOG_ERROR("Failed to deserialize global stored value map.");
        }
    };

    world.database->getGlobalStoredValueMap(std::move(loadGlobalMap));
}

entt::entity LoadHelper::loadClientEntity(
    Sint64 accountID, const SerializedClientComponents& accountComponents,
    const SerializedClientComponents& worldComponents)
{
    using EngineTypes = EnginePersistedComponentTypes;
    using ProjectTypes = ProjectPersistedComponentTypes;

    // Build the lists of each component's position within the buffers.
    // Note: The entity doesn't exist yet, so any errors here will be logged
    //       with entt::null.
    std::vector<SerializedComponent> accountEngineComponents{};
    std::vector<SerializedComponent> accountProjectComponents{};
    std::vector<SerializedComponent> worldEngineComponents{};
    std::vector<SerializedComponent> worldProjectComponents{};
    if (!parseComponents(accountComponents.engineComponents,
                         accountEngineComponents, entt::null, "account engine")
        || !parseComponents(accountComponents.projectComponents,
                            accountProjectComponents, entt::null,
                            "account project")
        || !parseComponents(worldComponents.engineComponents,
                            worldEngineComponents, entt::null, "world engine")
        || !parseComponents(worldComponents.projectComponents,
                            worldProjectComponents, entt::null,
                            "world project")) {
        LOG_WARN("Failed to parse client entity data for accountID: %lld",
                 static_cast<long long>(accountID));
        return entt::null;
    }

    // If there's a saved Position and GraphicState, deserialize them.
    // Note: We do this separately because we need them to pick the entity's
    //       position before creating it.
    std::optional<Position> savedPosition{};
    std::optional<GraphicState> savedGraphicState{};
    if (!getSavedClientComponent(accountEngineComponents, worldEngineComponents,
                                 savedPosition)
        || !getSavedClientComponent(accountEngineComponents,
                                    worldEngineComponents, savedGraphicState)) {
        LOG_WARN("Failed to load client entity Position or GraphicState for "
                 "accountID: %lld",
                 static_cast<long long>(accountID));
        return entt::null;
    }

    // If the entity's collision wouldn't be within the tile map at its saved
    // position (e.g. the map shrunk), use the spawn point instead.
    // Note: createEntity() fails safely if the position is outside of the
    //       map, but the graphics (and collision) are added later while
    //       loading components, where a failure would fail the whole load.
    //       Checking the collision up front lets us fall back instead. It
    //       also covers the position, since it's the collision's bottom
    //       center.
    // Note: If there's no saved GraphicState, the entity will be given the
    //       default graphic set.
    if (savedPosition) {
        const EntityGraphicSet& graphicSet{
            savedGraphicState ? graphicData.getEntityGraphicSet(
                                    savedGraphicState->graphicSetID)
                              : graphicData.getEntityGraphicSet(
                                    SharedConfig::DEFAULT_ENTITY_GRAPHIC_SET)};
        BoundingBox worldBounds{Transforms::modelToWorldEntity(
            graphicSet.getCollisionModelBounds(), *savedPosition)};
        if (!(world.tileMap.getTileExtent().contains(worldBounds))) {
            LOG_WARN("Entity wouldn't be within the tile map at its saved "
                     "position, using spawn point. accountID: %lld",
                     static_cast<long long>(accountID));
            savedPosition.reset();
        }
    }

    // Add the entity to the registry. If there's no saved position, use the
    // spawn point.
    entt::entity entity{world.createEntity(
        savedPosition ? *savedPosition : world.getSpawnPoint())};
    if (entity == entt::null) {
        LOG_ERROR("Failed to create client entity for accountID: %lld",
                  static_cast<long long>(accountID));
        return entt::null;
    }

    // Load each set's persisted components into the registry.
    using AccountEngineEntries
        = EngineTypes::ClientEntryTypes<ClientPersistence::Account>;
    using AccountProjectEntries
        = ProjectTypes::ClientEntryTypes<ClientPersistence::Account>;
    using WorldEngineEntries
        = EngineTypes::ClientEntryTypes<ClientPersistence::World>;
    using WorldProjectEntries
        = ProjectTypes::ClientEntryTypes<ClientPersistence::World>;
    bool loadSucceeded{loadEngineComponents<AccountEngineEntries>(
                           entity, accountEngineComponents)
                       && loadProjectComponents<AccountProjectEntries>(
                           entity, accountProjectComponents)
                       && loadEngineComponents<WorldEngineEntries>(
                           entity, worldEngineComponents)
                       && loadProjectComponents<WorldProjectEntries>(
                           entity, worldProjectComponents)};
    if (!loadSucceeded) {
        LOG_WARN("Failed to load client entity components for accountID: "
                 "%lld",
                 static_cast<long long>(accountID));
        world.registry.destroy(entity);
        return entt::null;
    }

    // Init any components with lazy-updated timers.
    // Note: If there's no SaveTimestamp, the account had no saved data in its
    //       set, so there are no timer components to init.
    if (world.registry.all_of<SaveTimestamp>(entity)) {
        initTimerComponents(entity);
    }

    return entity;
}

void LoadHelper::loadEntity(
    entt::entity entity, std::span<const Uint8> serializedEngineComponents,
    std::span<const Uint8> serializedProjectComponents)
{
    // Note: Any failure to load this entity is fatal. Unlike client entities,
    //       there's no way to recover (e.g. by rejecting the client), and if
    //       we kept going, the next save would overwrite the entity's data
    //       with whatever we managed to load. Failing at startup means
    //       nothing has been saved yet, so the data can still be fixed.
    // TODO: If we ever want the world to start despite bad entity data,
    //       quarantine the entity's saved data (e.g. move it to a separate
    //       table) instead of failing.

    // Build the lists of each component's position within the buffer.
    // Note: These lists are expected to have been saved in sorted order by
    //       type ID (the save path does this automatically).
    std::vector<SerializedComponent> engineComponents{};
    std::vector<SerializedComponent> projectComponents{};
    if (!parseComponents(serializedEngineComponents, engineComponents, entity,
                         "engine")
        || !parseComponents(serializedProjectComponents, projectComponents,
                            entity, "project")) {
        LOG_FATAL("Failed to parse saved data for entity %u.", entity);
    }

    // Find the Position component within the engine list and deserialize it.
    // Note: We do this separately because we know every entity has a
    //       Position, and we need it for createEntity() (and we want to
    //       use createEntity() to centralize logic and avoid bugs).
    using PositionEntry = EnginePersistedComponentTypes::EntryFor<Position>;
    const SerializedComponent* serializedPosition{
        findComponent(engineComponents, PositionEntry::TYPE_ID)};
    if (!serializedPosition) {
        LOG_FATAL("Tried to load entity %u with no Position.", entity);
    }

    Position position{};
    if (!deserializeComponent<PositionEntry>(entity, *serializedPosition,
                                             position)) {
        LOG_FATAL("Failed to load Position for entity %u.", entity);
    }

    // Add the entity to the registry.
    entt::entity newEntity{world.createEntity(position, entity)};
    if (newEntity != entity) {
        LOG_FATAL("Created entity ID doesn't match saved entity ID. "
                  "Created: %u, saved: %u",
                  newEntity, entity);
    }

    // Load the entity's persisted components into the registry.
    // Note: We load both sets before failing, so every component that
    //       failed gets logged.
    bool engineSucceeded{
        loadEngineComponents<EnginePersistedComponentTypes::EntryTypes>(
            newEntity, engineComponents)};
    bool projectSucceeded{
        loadProjectComponents<ProjectPersistedComponentTypes::EntryTypes>(
            newEntity, projectComponents)};
    if (!engineSucceeded || !projectSucceeded) {
        LOG_FATAL("Failed to load components for entity %u.", entity);
    }

    // Init any components with lazy-updated timers.
    initTimerComponents(newEntity);
}

bool LoadHelper::parseComponents(
    std::span<const Uint8> serializedComponents,
    std::vector<SerializedComponent>& components, entt::entity entity,
    const char* componentSetName) const
{
    // Build the list of components for this entity.
    // Note: This only stores each component's type/version/payload, we aren't
    //       doing anything heavy like allocating deserialized components.
    std::size_t readOffset{0};
    while (readOffset < serializedComponents.size()) {
        std::size_t remainingSize{serializedComponents.size() - readOffset};
        if (remainingSize < COMPONENT_HEADER_SIZE) {
            LOG_WARN("Malformed %s component data for entity %u: "
                     "%zu trailing bytes is too small for a component header.",
                     componentSetName, entity, remainingSize);
            return false;
        }

        const Uint8* header{serializedComponents.data() + readOffset};
        Uint16 typeID{ByteTools::read16(header + TYPE_ID_OFFSET)};
        Uint16 version{ByteTools::read16(header + VERSION_OFFSET)};
        Uint32 payloadSize{ByteTools::read32(header + PAYLOAD_SIZE_OFFSET)};
        readOffset += COMPONENT_HEADER_SIZE;

        remainingSize = serializedComponents.size() - readOffset;
        if (payloadSize > remainingSize) {
            LOG_WARN("Malformed %s component data for entity %u: component "
                     "ID %u declares a %u-byte payload, but only %zu bytes "
                     "remain.",
                     componentSetName, entity, typeID, payloadSize,
                     remainingSize);
            return false;
        }

        if (!components.empty() && typeID <= components.back().typeID) {
            LOG_WARN("Malformed %s component data for entity %u: component "
                     "IDs must be unique and ordered by ascending ID.",
                     componentSetName, entity);
            return false;
        }

        components.push_back(
            {typeID, version,
             serializedComponents.subspan(readOffset, payloadSize)});
        readOffset += payloadSize;
    }

    return true;
}

template<typename EntryList>
bool LoadHelper::loadEngineComponents(
    entt::entity entity,
    const std::vector<SerializedComponent>& serializedComponents)
{
    return loadComponents<EntryList>(
        entity, serializedComponents, [&](const auto& component) {
            using Component = std::decay_t<decltype(component)>;
            if constexpr (std::is_same_v<Component, Input>) {
                // Note: We don't use the persisted Input state, but we persist
                //       it to flag that the entity is movement-enabled.
                world.addMovementComponents(entity);
            }
            else if constexpr (std::is_same_v<Component, Rotation>) {
                // Note: If movement or graphics components are
                //       added first, this will be a replace.
                world.registry.emplace_or_replace<Rotation>(entity, component);
            }
            else if constexpr (std::is_same_v<Component, CollisionBitSets>) {
                if (world.registry.all_of<Collision>(entity)) {
                    // Note: If graphics components are added first, this will
                    //       be a replace.
                    world.registry.emplace_or_replace<CollisionBitSets>(
                        entity, component);
                }
                else {
                    LOG_WARN("Can't load CollisionBitSets for entity %u: "
                             "entity has no Collision "
                             "(addGraphicsComponents failed?)",
                             entity);
                    return false;
                }
            }
            else if constexpr (std::is_same_v<Component, GraphicState>) {
                // Note: We only persist GraphicState, but it implies the rest
                //       of the graphics components.
                if (!world.addGraphicsComponents(entity, component)) {
                    LOG_WARN(
                        "Failed to load graphics components for entity: %u",
                        entity);
                    return false;
                }
            }
            else {
                world.registry.emplace<Component>(entity, component);
            }

            return true;
        });
}

template<typename EntryList>
bool LoadHelper::loadProjectComponents(
    entt::entity entity,
    const std::vector<SerializedComponent>& serializedComponents)
{
    return loadComponents<EntryList>(
        entity, serializedComponents, [&](const auto& component) {
            using Component = std::decay_t<decltype(component)>;
            world.registry.emplace<Component>(entity, component);
            return true;
        });
}

template<typename EntryList, typename ComponentCallback>
bool LoadHelper::loadComponents(
    entt::entity entity,
    const std::vector<SerializedComponent>& serializedComponents,
    ComponentCallback&& componentCallback)
{
    bool allSucceeded{true};
    std::size_t serializedIndex{0};
    boost::mp11::mp_for_each<EntryList>(
        [&](auto entry) {
            using Entry = decltype(entry);

            // Skip serialized entries with unknown/deprecated IDs.
            while ((serializedIndex < serializedComponents.size())
                   && (serializedComponents[serializedIndex].typeID
                       < Entry::TYPE_ID)) {
                ++serializedIndex;
            }

            if (serializedIndex == serializedComponents.size()) {
                // No more serialized components to process.
                return;
            }

            const SerializedComponent& serializedComponent{
                serializedComponents[serializedIndex]};
            if (serializedComponent.typeID > Entry::TYPE_ID) {
                // The entity doesn't have this component, continue to the
                // next entry.
                return;
            }

            // Note: We skip Position since it's handled elsewhere.
            using Component = typename Entry::Component;
            if constexpr (!std::is_same_v<Component, Position>) {
                Component component{};
                if (!deserializeComponent<Entry>(entity, serializedComponent,
                                                 component)
                    || !componentCallback(component)) {
                    allSucceeded = false;
                }
            }

            ++serializedIndex;
        });

    return allSucceeded;
}

template<typename Entry>
bool LoadHelper::deserializeComponent(
    entt::entity entity, const SerializedComponent& serializedComponent,
    typename Entry::Component& component)
{
    using Component = typename Entry::Component;
    constexpr auto componentName{entt::type_name<Component>::value()};

    if (serializedComponent.version > Entry::VERSION) {
        LOG_WARN("Can't load component %.*s (ID: %u) for entity %u: "
                 "serialized version %u is newer than current version %u.",
                 static_cast<int>(componentName.size()), componentName.data(),
                 serializedComponent.typeID, entity,
                 serializedComponent.version, Entry::VERSION);
        return false;
    }

    if (serializedComponent.version < Entry::VERSION) {
        if constexpr (Entry::VERSION > 0) {
            ComponentMigrationResult result{
                migrateComponent<Component, Entry::VERSION>(
                    serializedComponent.version, serializedComponent.payload,
                    component)};
            if (result == ComponentMigrationResult::Success) {
                return true;
            }

            if (result == ComponentMigrationResult::DeserializationFailed) {
                LOG_WARN("Failed to deserialize component %.*s (ID: %u) "
                         "version %u while migrating it to version %u for "
                         "entity %u.",
                         static_cast<int>(componentName.size()),
                         componentName.data(), serializedComponent.typeID,
                         serializedComponent.version, Entry::VERSION, entity);
            }
            else if (result == ComponentMigrationResult::MigrationFailed) {
                LOG_WARN("Migration failed for component %.*s (ID: %u) from "
                         "version %u to version %u for entity %u.",
                         static_cast<int>(componentName.size()),
                         componentName.data(), serializedComponent.typeID,
                         serializedComponent.version, Entry::VERSION, entity);
            }
            else {
                LOG_WARN("No migration path for component %.*s (ID: %u) "
                         "from version %u to version %u for entity %u.",
                         static_cast<int>(componentName.size()),
                         componentName.data(), serializedComponent.typeID,
                         serializedComponent.version, Entry::VERSION, entity);
            }
        }
        else {
            LOG_WARN("Can't migrate component %.*s (ID: %u) for entity %u: "
                     "current component version is 0.",
                     static_cast<int>(componentName.size()),
                     componentName.data(), serializedComponent.typeID, entity);
        }
        return false;
    }

    if (!Deserialize::fromBuffer(serializedComponent.payload.data(),
                                 serializedComponent.payload.size(),
                                 component)) {
        LOG_WARN("Failed to deserialize component %.*s (ID: %u, version: "
                 "%u) for entity %u.",
                 static_cast<int>(componentName.size()), componentName.data(),
                 serializedComponent.typeID, serializedComponent.version,
                 entity);
        return false;
    }

    return true;
}

void LoadHelper::initTimerComponents(entt::entity entity)
{
    // Timer components are updated using SaveTimestamp, so client entities
    // must save them in the same set (otherwise, a timer could be loaded with
    // a SaveTimestamp from a different save).
    using EngineTypes = EnginePersistedComponentTypes;
    static_assert(
        EngineTypes::EntryFor<CastCooldown>::CLIENT_PERSISTENCE
            == EngineTypes::EntryFor<SaveTimestamp>::CLIENT_PERSISTENCE,
        "CastCooldown must have the same ClientPersistence as SaveTimestamp.");

    const SaveTimestamp* saveTimestamp{
        world.registry.try_get<SaveTimestamp>(entity)};
    if (!saveTimestamp) {
        LOG_ERROR("Entity was just loaded but has no SaveTimestamp.");
        return;
    }

    // Init CastCooldown, if present.
    if (CastCooldown * castCooldown{
            world.registry.try_get<CastCooldown>(entity)}) {
        castCooldown->initAfterLoad(saveTimestamp->lastSavedTick,
                                    simulation.getCurrentTick());
    }
}

const LoadHelper::SerializedComponent* LoadHelper::findComponent(
    const std::vector<SerializedComponent>& serializedComponents,
    Uint16 typeID) const
{
    // Note: serializedComponents is sorted by type ID (parseComponents()
    //       enforces it).
    auto componentIt{std::lower_bound(
        serializedComponents.begin(), serializedComponents.end(), typeID,
        [](const SerializedComponent& component, Uint16 typeID) {
            return component.typeID < typeID;
        })};
    if ((componentIt == serializedComponents.end())
        || (componentIt->typeID != typeID)) {
        return nullptr;
    }

    return &(*componentIt);
}

template<typename Component>
bool LoadHelper::getSavedClientComponent(
    const std::vector<SerializedComponent>& accountEngineComponents,
    const std::vector<SerializedComponent>& worldEngineComponents,
    std::optional<Component>& component)
{
    using Entry = EnginePersistedComponentTypes::EntryFor<Component>;
    if constexpr (Entry::CLIENT_PERSISTENCE == ClientPersistence::None) {
        // Client entities don't save this component.
        return true;
    }
    else {
        const std::vector<SerializedComponent>& engineComponents{
            (Entry::CLIENT_PERSISTENCE == ClientPersistence::Account)
                ? accountEngineComponents
                : worldEngineComponents};
        const SerializedComponent* serializedComponent{
            findComponent(engineComponents, Entry::TYPE_ID)};
        if (!serializedComponent) {
            return true;
        }

        // Note: The entity doesn't exist yet, so any errors will be logged
        //       with entt::null.
        component.emplace();
        if (!deserializeComponent<Entry>(entt::null, *serializedComponent,
                                         *component)) {
            component.reset();
            return false;
        }

        return true;
    }
}

} // namespace WorldServer
} // namespace AM
