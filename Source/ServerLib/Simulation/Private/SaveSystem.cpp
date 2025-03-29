#include "SaveSystem.h"
#include "World.h"
#include "ItemData.h"
#include "Config.h"
#include "Database.h"
#include "EnginePersistedComponentTypes.h"
#include "ProjectPersistedComponentTypes.h"
#include "ClientSimData.h"
#include "Serialize.h"
#include "Log.h"
#include "boost/mp11/algorithm.hpp"
#include <type_traits>

namespace AM
{
namespace Server
{

/**
 * Retrieves all persisted component types that the entity possesses from 
 * TypeList and pushes them into componentVec.
 *
 * Note: This is a free function to reduce includes in the header.
 */
template<typename TypeList, typename VariantType>
void addComponentsToVector(entt::registry& registry, entt::entity entity,
                           std::vector<VariantType>& componentVec)
{
    boost::mp11::mp_for_each<TypeList>([&](auto I) {
        using ComponentType = decltype(I);

        if (registry.all_of<ComponentType>(entity)) {
            if constexpr (std::is_empty_v<ComponentType>) {
                // Note: Can't registry.get() empty types.
                componentVec.push_back(ComponentType{});
            }
            else {
                componentVec.push_back(registry.get<ComponentType>(entity));
            }
        }
    });
}

SaveSystem::SaveSystem(World& inWorld, ItemData& inItemData)
: world{inWorld}
, itemData{inItemData}
, updatedItems{}
, saveTimer{}
, workBuffer1{}
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
    auto view{
        world.registry.view<entt::entity>(entt::exclude_t<ClientSimData>{})};
    std::vector<EnginePersistedComponent> engineComponents{};
    std::vector<ProjectPersistedComponent> projectComponents{};
    for (entt::entity entity : view) {
        engineComponents.clear();
        projectComponents.clear();

        addComponentsToVector<EnginePersistedComponentTypes,
                              EnginePersistedComponent>(
            world.registry, entity, engineComponents);
        addComponentsToVector<ProjectPersistedComponentTypes,
                              ProjectPersistedComponent>(
            world.registry, entity, projectComponents);

        workBuffer1.clear();
        workBuffer2.clear();
        workBuffer1.resize(Serialize::measureSize(engineComponents));
        workBuffer2.resize(Serialize::measureSize(projectComponents));
        Serialize::toBuffer(workBuffer1.data(), workBuffer1.size(),
                            engineComponents);
        Serialize::toBuffer(workBuffer2.data(), workBuffer2.size(),
                            projectComponents);

        world.database->saveEntityData(entity, workBuffer1, workBuffer2);
    }
}

void SaveSystem::saveItems()
{
    // Remove duplicates from the vector.
    std::sort(updatedItems.begin(), updatedItems.end());
    updatedItems.erase(
        std::unique(updatedItems.begin(), updatedItems.end()),
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
