#include "SaveHelper.h"
#include "World.h"
#include "Simulation.h"
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
namespace WorldServer
{
SaveHelper::SaveHelper(World& inWorld, Simulation& inSimulation)
: world{inWorld}
, simulation{inSimulation}
, workBuffer1{}
, workBuffer2{}
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
}

void SaveHelper::saveClientEntity(entt::entity entity)
{
    // Update the entity's SaveTimestamp.
    SaveTimestamp& saveTimestamp{
        world.registry.get_or_emplace<SaveTimestamp>(entity)};
    saveTimestamp.lastSavedTick = simulation.getCurrentTick();

    // Save the entity's data.
    Sint64 accountID{world.registry.get<ClientSimData>(entity).accountID};
    saveClientComponents<ClientPersistence::Account>(entity, accountID);
    saveClientComponents<ClientPersistence::World>(entity, accountID);
}

template<ClientPersistence Persistence>
void SaveHelper::saveClientComponents(entt::entity entity, Sint64 accountID)
{
    using EngineTypes = EnginePersistedComponentTypes;
    using ProjectTypes = ProjectPersistedComponentTypes;

    // If no components use this persistence, there's nothing to save.
    if constexpr (EngineTypes::HAS_CLIENT_ENTRIES<Persistence>
                  || ProjectTypes::HAS_CLIENT_ENTRIES<Persistence>) {
        serializeComponents<EngineTypes::ClientEntryTypes<Persistence>>(
            entity, workBuffer1);
        serializeComponents<ProjectTypes::ClientEntryTypes<Persistence>>(
            entity, workBuffer2);

        world.database->saveClientEntityData(Persistence, accountID,
                                             workBuffer1, workBuffer2);
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
