#include "SaveSystem.h"
#include "World.h"
#include "Config.h"
#include "PersistedEntityData.h"
#include "Database.h"
#include "ClientSimData.h"
#include "Serialize.h"
#include "Log.h"
#include <type_traits>

namespace AM
{
namespace Server
{

/**
 * Retrieves all persisted component types that entity possesses and pushes
 * them into componentVec.
 *
 * Note: This is a free function to reduce includes in the header.
 */
void addComponentsToVector(entt::registry& registry, entt::entity entity,
                           std::vector<PersistedComponent>& componentVec)
{
    boost::mp11::mp_for_each<PersistedComponentTypes>([&](auto I) {
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

SaveSystem::SaveSystem(World& inWorld)
: world{inWorld}
, updatedItems{}
, saveTimer{}
, workBuffer{}
{
    // When an item is created or updated, add it to updatedItems.
    world.itemData.itemCreated.connect<&SaveSystem::itemUpdated>(this);
    world.itemData.itemUpdated.connect<&SaveSystem::itemUpdated>(this);
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
    for (entt::entity entity : view) {
        PersistedEntityData persistedEntityData{};
        addComponentsToVector(world.registry, entity,
                              persistedEntityData.components);

        workBuffer.clear();
        workBuffer.resize(Serialize::measureSize(persistedEntityData));
        Serialize::toBuffer(workBuffer.data(), workBuffer.size(),
                            persistedEntityData);

        world.database->saveEntityData(entity, workBuffer.data(),
                                       workBuffer.size());
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
        const Item* updatedItem{world.itemData.getItem(itemID)};

        workBuffer.clear();
        workBuffer.resize(Serialize::measureSize(*updatedItem));
        Serialize::toBuffer(workBuffer.data(), workBuffer.size(), *updatedItem);

        world.database->saveItemData(updatedItem->numericID, workBuffer.data(),
                                     workBuffer.size());
    }

    updatedItems.clear();
}

void SaveSystem::saveStoredValues()
{
    // Serialize the entity stored value ID map.
    workBuffer.clear();
    workBuffer.resize(Serialize::measureSize(world.entityStoredValueIDMap));
    Serialize::toBuffer(workBuffer.data(), workBuffer.size(),
                        world.entityStoredValueIDMap);

    // Queue the save query.
    world.database->saveEntityStoredValueIDMap(workBuffer.data(),
                                               workBuffer.size());

    // Serialize the global stored value map.
    workBuffer.clear();
    workBuffer.resize(Serialize::measureSize(world.globalStoredValueMap));
    Serialize::toBuffer(workBuffer.data(), workBuffer.size(),
                        world.globalStoredValueMap);

    // Queue the save query.
    world.database->saveGlobalStoredValueMap(workBuffer.data(),
                                             workBuffer.size());
}

} // namespace Server
} // namespace AM
