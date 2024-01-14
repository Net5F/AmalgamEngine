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
, saveTimer{}
, mapHasBeenSaved{}
, nceHaveBeenSaved{}
{
}

void SaveSystem::saveIfNecessary()
{
    // We stagger the save times so that they don't cause a performance spike.
    static constexpr float MAP_SAVE_TIME_S{Config::SAVE_PERIOD_S * (1.0/3.0)};
    static constexpr float NCE_SAVE_TIME_S{Config::SAVE_PERIOD_S * (2.0/3.0)};
    static constexpr float ITEM_SAVE_TIME_S{Config::SAVE_PERIOD_S * (3.0/3.0)};

    // If enough time has passed, save the map state to TileMap.bin.
    //if (!mapHasBeenSaved && (saveTimer.getTime() >= MAP_SAVE_TIME_S)) {
    //    world.tileMap.save("TileMap.bin");

    //    mapHasBeenSaved = true;
    //}

    // If enough time has passed, save the non-client entity state to the 
    // database.
    //if (!nceHaveBeenSaved && (saveTimer.getTime() >= NCE_SAVE_TIME_S)) {
    if (!nceHaveBeenSaved && (saveTimer.getTime() >= 20)) {
        saveNonClientEntities();
        saveTimer.reset();

        //nceHaveBeenSaved = true;
    }

    // If enough time has passed, save the item definitions to the database.
    //if (saveTimer.getTime() >= ITEM_SAVE_TIME_S) {

    //    saveTimer.reset();
    //}
}

void SaveSystem::saveNonClientEntities()
{
    LOG_INFO("Saving non-client entities...");

    // Track some stats.
    Timer timer;
    std::size_t entityCount{0};

    // Start the transaction.
    world.database->startTransaction();

    // Queue all of our entity save queries.
    auto view{
        world.registry.view<entt::entity>(entt::exclude_t<ClientSimData>{})};
    BinaryBuffer buffer{};
    for (entt::entity entity : view) {
        PersistedEntityData persistedEntityData{entity};
        addComponentsToVector(world.registry, entity,
                              persistedEntityData.components);

        buffer.resize(Serialize::measureSize(persistedEntityData));
        Serialize::toBuffer(buffer.data(), buffer.size(), persistedEntityData);

        world.database->saveEntityData(entity, buffer.data(), buffer.size());

        entityCount++;
    }

    // Commit the transaction.
    world.database->commitTransaction();

    // Print the time taken.
    double timeTaken{timer.getTime()};
    LOG_INFO("Saved %u non-client entities in %.6fs.", entityCount, timeTaken);
}

} // namespace Server
} // namespace AM
