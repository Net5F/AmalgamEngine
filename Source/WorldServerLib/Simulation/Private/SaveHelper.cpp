#include "SaveHelper.h"
#include "World.h"
#include "Simulation.h"
#include "Database.h"
#include "EnginePersistedComponentTypes.h"
#include "ProjectPersistedComponentTypes.h"
#include "ClientPersistenceUsage.h"
#include "AccountSaveQueue.h"
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
#include <utility>

namespace AM
{
namespace WorldServer
{
SaveHelper::SaveHelper(World& inWorld, Simulation& inSimulation)
: world{inWorld}
, simulation{inSimulation}
, workBuffer1{}
, workBuffer2{}
, failedWorldSaves{}
{
}

void SaveHelper::saveNonClientEntities()
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
        serializeComponents<EnginePersistedComponentTypes::EntryTypes>(
            entity, workBuffer1);
        serializeComponents<ProjectPersistedComponentTypes::EntryTypes>(
            entity, workBuffer2);

        world.database->saveEntityData(entity, workBuffer1, workBuffer2);
    }
}

void SaveHelper::saveClientEntities()
{
    // Note: Lingering entities keep their ClientSimData, so this includes
    //       them.
    for (entt::entity entity : world.registry.view<ClientSimData>()) {
        saveClientEntity(entity);
    }

    // Retry any saves that failed (including those of entities that have
    // since been removed).
    retryFailedWorldSaves();
}

void SaveHelper::saveClientEntity(entt::entity entity)
{
    // Update the entity's SaveTimestamp.
    SaveTimestamp& saveTimestamp{
        world.registry.get_or_emplace<SaveTimestamp>(entity)};
    saveTimestamp.lastSavedTick = simulation.getCurrentTick();

    // Save the entity's data.
    Sint64 accountID{world.registry.get<ClientSimData>(entity).accountID};
    saveAccountComponents(entity, accountID);
    saveWorldComponents(entity, accountID);
}

const SaveHelper::ClientWorldData*
    SaveHelper::getUnsavedWorldData(Sint64 accountID) const
{
    auto saveIt{failedWorldSaves.find(accountID)};
    if (saveIt == failedWorldSaves.end()) {
        return nullptr;
    }

    return &(saveIt->second);
}

void SaveHelper::saveAccountComponents(entt::entity entity, Sint64 accountID)
{
    constexpr ClientPersistence Persistence{ClientPersistence::Account};

    // If no components use this persistence, there's nothing to save.
    if constexpr (USES_CLIENT_PERSISTENCE<Persistence>) {
        // Note: We serialize directly into the AccountData, since the queue
        //       takes ownership of it.
        AccountData accountData{};
        serializeComponents<
            EnginePersistedComponentTypes::ClientEntryTypes<Persistence>>(
            entity, accountData.engineComponents);
        serializeComponents<
            ProjectPersistedComponentTypes::ClientEntryTypes<Persistence>>(
            entity, accountData.projectComponents);

        world.accountSaveQueue.queueSave(accountID, std::move(accountData));
    }
}

void SaveHelper::saveWorldComponents(entt::entity entity, Sint64 accountID)
{
    constexpr ClientPersistence Persistence{ClientPersistence::World};

    // If no components use this persistence, there's nothing to save.
    if constexpr (USES_CLIENT_PERSISTENCE<Persistence>) {
        serializeComponents<
            EnginePersistedComponentTypes::ClientEntryTypes<Persistence>>(
            entity, workBuffer1);
        serializeComponents<
            ProjectPersistedComponentTypes::ClientEntryTypes<Persistence>>(
            entity, workBuffer2);

        // If the data is too large, skip the save. Keep whatever data is
        // already saved (and any failed save, since it was within the limit).
        if ((workBuffer1.size() > CLIENT_WORLD_DATA_BLOB_MAX_BYTES)
            || (workBuffer2.size() > CLIENT_WORLD_DATA_BLOB_MAX_BYTES)) {
            LOG_ERROR("World data exceeds size limit, skipping save. "
                      "AccountID: %lld, engine bytes: %zu, project bytes: %zu",
                      static_cast<long long>(accountID), workBuffer1.size(),
                      workBuffer2.size());
            return;
        }

        // If the save succeeds, it replaces any data that previously failed.
        // If it fails, keep the data so it isn't lost.
        if (world.database->saveClientEntityData(accountID, workBuffer1,
                                                 workBuffer2)) {
            failedWorldSaves.erase(accountID);
        }
        else {
            failedWorldSaves[accountID]
                = ClientWorldData{workBuffer1, workBuffer2};
        }
    }
}

void SaveHelper::retryFailedWorldSaves()
{
    for (auto it = failedWorldSaves.begin(); it != failedWorldSaves.end();) {
        const ClientWorldData& data{it->second};
        if (world.database->saveClientEntityData(
                it->first, data.engineComponents, data.projectComponents)) {
            it = failedWorldSaves.erase(it);
        }
        else {
            ++it;
        }
    }

    if (!(failedWorldSaves.empty())) {
        LOG_ERROR("%zu client entities' World data failed to save. It will "
                  "be retried, but will be lost if the server exits first.",
                  failedWorldSaves.size());
    }
}

template<typename EntryList>
void SaveHelper::serializeComponents(entt::entity entity,
                                     BinaryBuffer& outputBuffer)
{
    entt::registry& registry{world.registry};

    outputBuffer.clear();
    boost::mp11::mp_for_each<EntryList>([&](auto entry) {
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

template<typename Entry>
void SaveHelper::serializeComponent(typename Entry::Component& component,
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

    std::size_t bytesWritten{Serialize::toBuffer(
        outputBuffer.data(), outputBuffer.size(), component, payloadOffset)};
    if (bytesWritten != payloadSize) {
        outputBuffer.resize(recordOffset);
        LOG_ERROR("Failed to save component %.*s (ID: %u, version: %u): "
                  "measured size was %zu bytes, but serialization wrote %zu.",
                  static_cast<int>(componentName.size()), componentName.data(),
                  Entry::TYPE_ID, Entry::VERSION, payloadSize, bytesWritten);
    }
}

} // namespace WorldServer
} // namespace AM
