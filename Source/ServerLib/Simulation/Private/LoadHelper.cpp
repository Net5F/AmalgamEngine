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
#include "Deserialize.h"
#include "ByteTools.h"
#include "Log.h"
#include "entt/core/type_info.hpp"
#include "boost/mp11/algorithm.hpp"
#include <algorithm>
#include <functional>
#include <span>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace AM
{
namespace Server
{

LoadHelper::LoadHelper(World& inWorld, Simulation& inSimulation,
                       ItemData& inItemData)
: world{inWorld}
, simulation{inSimulation}
, itemData{inItemData}
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

void LoadHelper::loadStoredValues()
{
    // Load the entity stored value IDs.
    auto loadEntityMap = [&](std::span<const Uint8> serializedMap) {
        if (serializedMap.empty()) {
            return;
        }

        if (!Deserialize::fromBuffer(serializedMap.data(), serializedMap.size(),
                                     world.entityStoredValueIDMap)) {
            LOG_ERROR("Failed to deserialize entity stored value ID map.");
        }
    };

    world.database->getEntityStoredValueIDMap(std::move(loadEntityMap));

    // Load the global stored values.
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

void LoadHelper::loadEntity(
    entt::entity entity, std::span<const Uint8> serializedEngineComponents,
    std::span<const Uint8> serializedProjectComponents)
{
    // Build the lists of each component's position within the buffer.
    // Note: These lists are expected to have been saved in sorted order by 
    //       type ID (the save path does this automatically).
    std::vector<SerializedComponent> engineComponents{};
    std::vector<SerializedComponent> projectComponents{};
    if (!parseComponents(serializedEngineComponents, engineComponents, entity,
                         "engine")) {
        return;
    }
    if (!parseComponents(serializedProjectComponents, projectComponents,
                         entity, "project")) {
        return;
    }

    // Find the Position component within the engine list and deserialize it.
    // Note: We do this separately because we know every entity has a
    //       Position, and we need it for createEntity() (and we want to
    //       use createEntity() to centralize logic and avoid bugs).
    using PositionEntry
        = EnginePersistedComponentTypes::EntryFor<Position>;
    auto positionIt{std::lower_bound(
        engineComponents.begin(), engineComponents.end(),
        PositionEntry::TYPE_ID,
        [](const SerializedComponent& component, Uint16 typeID) {
            return component.typeID < typeID;
        })};
    if ((positionIt == engineComponents.end())
        || (positionIt->typeID != PositionEntry::TYPE_ID)) {
        LOG_ERROR("Tried to load entity %u with no Position. Skipping.",
                  entity);
        return;
    }

    Position position{};
    if (!deserializeComponent<PositionEntry>(entity, *positionIt, position)) {
        return;
    }

    // Add the entity to the registry.
    entt::entity newEntity{world.createEntity(position, entity)};
    if (newEntity != entity) {
        LOG_FATAL("Created entity ID doesn't match saved entity ID. "
                  "Created: %u, saved: %u",
                  newEntity, entity);
    }

    // Load the entity's persisted components into the registry.
    loadEngineComponents(newEntity, engineComponents);
    loadProjectComponents(newEntity, projectComponents);

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
            LOG_ERROR("Malformed %s component data for entity %u: "
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
            LOG_ERROR("Malformed %s component data for entity %u: component "
                      "ID %u declares a %u-byte payload, but only %zu bytes "
                      "remain.",
                      componentSetName, entity, typeID, payloadSize,
                      remainingSize);
            return false;
        }

        if (!components.empty() && typeID <= components.back().typeID) {
            LOG_ERROR("Malformed %s component data for entity %u: component "
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

void LoadHelper::loadEngineComponents(
    entt::entity entity,
    const std::vector<SerializedComponent>& serializedComponents)
{
    loadComponents<EnginePersistedComponentTypes>(
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
                    LOG_ERROR("Can't load CollisionBitSets for entity %u: "
                              "entity has no Collision "
                              "(addGraphicsComponents failed?)",
                              entity);
                }
            }
            else if constexpr (std::is_same_v<Component, GraphicState>) {
                // Note: We only persist GraphicState, but it implies the rest
                //       of the graphics components.
                if (!world.addGraphicsComponents(entity, component)) {
                    LOG_ERROR(
                        "Failed to load graphics components for entity: %u",
                        entity);
                }
            }
            else {
                world.registry.emplace<Component>(entity, component);
            }
        });
}

void LoadHelper::loadProjectComponents(
    entt::entity entity,
    const std::vector<SerializedComponent>& serializedComponents)
{
    loadComponents<ProjectPersistedComponentTypes>(
        entity, serializedComponents, [&](const auto& component) {
            using Component = std::decay_t<decltype(component)>;
            world.registry.emplace<Component>(entity, component);
        });
}

template<typename ComponentList, typename ComponentCallback>
void LoadHelper::loadComponents(
    entt::entity entity,
    const std::vector<SerializedComponent>& serializedComponents,
    ComponentCallback&& componentCallback)
{
    std::size_t serializedIndex{0};
    boost::mp11::mp_for_each<typename ComponentList::EntryTypes>(
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
                if (deserializeComponent<Entry>(entity, serializedComponent,
                                                component)) {
                    componentCallback(component);
                }
            }

            ++serializedIndex;
        });
}

template<typename Entry>
bool LoadHelper::deserializeComponent(
    entt::entity entity, const SerializedComponent& serializedComponent,
    typename Entry::Component& component)
{
    using Component = typename Entry::Component;
    constexpr auto componentName{entt::type_name<Component>::value()};

    if (serializedComponent.version > Entry::VERSION) {
        LOG_ERROR("Can't load component %.*s (ID: %u) for entity %u: "
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

            if (result
                == ComponentMigrationResult::DeserializationFailed) {
                LOG_ERROR("Failed to deserialize component %.*s (ID: %u) "
                          "version %u while migrating it to version %u for "
                          "entity %u.",
                          static_cast<int>(componentName.size()),
                          componentName.data(), serializedComponent.typeID,
                          serializedComponent.version, Entry::VERSION, entity);
            }
            else if (result == ComponentMigrationResult::MigrationFailed) {
                LOG_ERROR("Migration failed for component %.*s (ID: %u) from "
                          "version %u to version %u for entity %u.",
                          static_cast<int>(componentName.size()),
                          componentName.data(), serializedComponent.typeID,
                          serializedComponent.version, Entry::VERSION, entity);
            }
            else {
                LOG_ERROR("No migration path for component %.*s (ID: %u) "
                          "from version %u to version %u for entity %u.",
                          static_cast<int>(componentName.size()),
                          componentName.data(), serializedComponent.typeID,
                          serializedComponent.version, Entry::VERSION, entity);
            }
        }
        else {
            LOG_ERROR("Can't migrate component %.*s (ID: %u) for entity %u: "
                      "current component version is 0.",
                      static_cast<int>(componentName.size()),
                      componentName.data(), serializedComponent.typeID, entity);
        }
        return false;
    }

    if (!Deserialize::fromBuffer(serializedComponent.payload.data(),
                                 serializedComponent.payload.size(),
                                 component)) {
        LOG_ERROR("Failed to deserialize component %.*s (ID: %u, version: "
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

} // namespace Server
} // namespace AM
