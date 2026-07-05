#include "SaveSystem.h"
#include "SimulationContext.h"
#include "Simulation.h"
#include "World.h"
#include "ItemData.h"
#include "Config.h"
#include "Database.h"
#include "EnginePersistedComponentTypes.h"
#include "ProjectPersistedComponentTypes.h"
#include "PersistedComponentDefs.h"
#include "ClientSimData.h"
#include "SaveTimestamp.h"
#include "Serialize.h"
#include "ByteTools.h"
#include "Log.h"
#include "boost/mp11/algorithm.hpp"
#include "entt/core/type_info.hpp"
#include <limits>
#include <type_traits>

namespace AM
{
namespace Server
{

/**
 * Appends Component to outputBuffer using the persisted component record
 * format.
 *
 * See PERSISTED_COMPONENTS_VERSION comment for format info.
 *
 * Note: This is a free function to reduce includes in the header.
 */
template<typename Entry>
void serializeComponent(typename Entry::Component& component,
                        BinaryBuffer& outputBuffer)
{
    using Component = typename Entry::Component;
    constexpr auto componentName{entt::type_name<Component>::value()};

    std::size_t payloadSize{Serialize::measureSize(component)};
    if (payloadSize > std::numeric_limits<Uint32>::max()) {
        LOG_ERROR("Can't save component %.*s (ID: %u, version: %u): "
                  "serialized size exceeds Uint32.",
                  static_cast<int>(componentName.size()), componentName.data(),
                  Entry::TYPE_ID, Entry::VERSION);
        return;
    }

    const std::size_t recordOffset{outputBuffer.size()};
    const std::size_t payloadOffset{recordOffset + COMPONENT_HEADER_SIZE};
    outputBuffer.resize(payloadOffset + payloadSize);

    Uint8* header{outputBuffer.data() + recordOffset};
    ByteTools::write16(Entry::TYPE_ID, header + TYPE_ID_OFFSET);
    ByteTools::write16(Entry::VERSION, header + VERSION_OFFSET);
    ByteTools::write32(static_cast<Uint32>(payloadSize),
                       header + PAYLOAD_SIZE_OFFSET);

    std::size_t bytesWritten{
        Serialize::toBuffer(outputBuffer.data(), outputBuffer.size(), component,
                            payloadOffset)};
    if (bytesWritten != payloadSize) {
        outputBuffer.resize(recordOffset);
        LOG_ERROR("Failed to save component %.*s (ID: %u, version: %u): "
                  "measured size was %zu bytes, but serialization wrote %zu.",
                  static_cast<int>(componentName.size()), componentName.data(),
                  Entry::TYPE_ID, Entry::VERSION, payloadSize, bytesWritten);
    }
}

/**
 * Serializes each component from ComponentList that entity possesses.
 *
 * Note: ComponentList requires its entries to be ordered by ascending ID, so 
 *       the serialized records will also be ordered.
 */
template<typename ComponentList>
void serializeComponents(entt::registry& registry, entt::entity entity,
                         BinaryBuffer& outputBuffer)
{
    outputBuffer.clear();
    boost::mp11::mp_for_each<typename ComponentList::EntryTypes>(
        [&](auto entry) {
            using Entry = decltype(entry);
            using Component = typename Entry::Component;

            if (!registry.all_of<Component>(entity)) {
                return;
            }

            if constexpr (std::is_empty_v<Component>) {
                // Note: Can't registry.get() empty types.
                Component component{};
                serializeComponent<Entry>(component, outputBuffer);
            }
            else {
                serializeComponent<Entry>(registry.get<Component>(entity),
                                          outputBuffer);
            }
        });
}

SaveSystem::SaveSystem(const SimulationContext& inSimContext)
: simulation{inSimContext.simulation}
, world{inSimContext.simulation.getWorld()}
, itemData{inSimContext.itemData}
, updatedItems{}
, saveTimer{}
, workBuffer1{}
, workBuffer2{}
{
    // When an item is created or updated, add it to updatedItems.
    itemData.itemCreated.connect<&SaveSystem::itemUpdated>(this);
    itemData.itemUpdated.connect<&SaveSystem::itemUpdated>(this);
}

void SaveSystem::saveIfNecessary()
{
    // If enough time has passed and a backup isn't still underway, save
    // everything to the database.
    if ((saveTimer.getTime() >= Config::SAVE_PERIOD_S)
        && !(world.database->backupIsInProgress())) {
        LOG_INFO("Saving entities, items, and map...");

        // Save all of our data to the in-memory database.
        world.database->startTransaction();

        saveNonClientEntities();
        saveItems();
        saveStoredValues();
        // TODO: Track changed tiles and save to the database.
        world.tileMap.save("TileMap.bin");

        world.database->commitTransaction();

        // Backup the in-memory database to the file database.
        world.database->backupToFile();

        saveTimer.reset();
    }
}

void SaveSystem::itemUpdated(ItemID itemID)
{
    updatedItems.emplace_back(itemID);
}

void SaveSystem::saveNonClientEntities()
{
    // Note: If doing a full save ever starts taking too long, we can add
    //       an observer that tracks changes to non-client entities and only
    //       save those.

    // Queue all of our entity save queries.
    Uint32 currentTick{simulation.getCurrentTick()};
    auto view{
        world.registry.view<entt::entity>(entt::exclude_t<ClientSimData>{})};
    for (entt::entity entity : view) {
        // Update the entity's SaveTimestamp.
        SaveTimestamp& saveTimestamp{
            world.registry.get_or_emplace<SaveTimestamp>(entity)};
        saveTimestamp.lastSavedTick = currentTick;

        // Save the entity's data.
        serializeComponents<EnginePersistedComponentTypes>(
            world.registry, entity, workBuffer1);
        serializeComponents<ProjectPersistedComponentTypes>(
            world.registry, entity, workBuffer2);

        world.database->saveEntityData(entity, workBuffer1, workBuffer2);
    }
}

void SaveSystem::saveItems()
{
    // Remove duplicates from the vector.
    std::sort(updatedItems.begin(), updatedItems.end());
    updatedItems.erase(std::unique(updatedItems.begin(), updatedItems.end()),
                       updatedItems.end());

    // Queue all of our item save queries.
    for (ItemID itemID : updatedItems) {
        const Item* updatedItem{itemData.getItem(itemID)};

        workBuffer1.clear();
        workBuffer1.resize(Serialize::measureSize(*updatedItem));
        Serialize::toBuffer(workBuffer1.data(), workBuffer1.size(),
                            *updatedItem);

        world.database->saveItemData(
            updatedItem->numericID, workBuffer1,
            itemData.getItemVersion(updatedItem->numericID),
            itemData.getItemInitScript(updatedItem->numericID).script);
    }

    updatedItems.clear();
}

void SaveSystem::saveStoredValues()
{
    // Serialize the entity stored value ID map.
    workBuffer1.clear();
    workBuffer1.resize(Serialize::measureSize(world.entityStoredValueIDMap));
    Serialize::toBuffer(workBuffer1.data(), workBuffer1.size(),
                        world.entityStoredValueIDMap);

    // Queue the save query.
    world.database->saveEntityStoredValueIDMap(workBuffer1);

    // Serialize the global stored value map.
    workBuffer1.clear();
    workBuffer1.resize(Serialize::measureSize(world.globalStoredValueMap));
    Serialize::toBuffer(workBuffer1.data(), workBuffer1.size(),
                        world.globalStoredValueMap);

    // Queue the save query.
    world.database->saveGlobalStoredValueMap(workBuffer1);
}

} // namespace Server
} // namespace AM
