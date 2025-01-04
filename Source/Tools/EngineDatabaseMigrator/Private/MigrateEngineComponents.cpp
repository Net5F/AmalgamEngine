#include "MigrateEngineComponents.h"
#include "EnginePersistedComponentTypes.h"
#include "Serialize.h"
#include "Deserialize.h"
#include "VariantTools.h"
#include "SQLiteCpp/SQLiteCpp.h"
#include "Log.h"
#include "entt/fwd.hpp"

// Note: This is all just example code used in testing. The test involved 
//       adding a "MyNewComponent" struct to the middle of the component type 
//       list.
//       If we end up doing this a lot, we should make some helper functions 
//       to make this process easier.
namespace AM
{
using EnginePersistedComponentTypesV0
    = boost::mp11::mp_list<Name, Position, Rotation, GraphicState, Interaction,
                           Server::ItemHandlers, Server::Dialogue,
                           EntityInitScript, Server::StoredValues>;
using EnginePersistedComponentV0
    = boost::mp11::mp_rename<EnginePersistedComponentTypesV0, std::variant>;
template<typename S>
void serialize(S& serializer,
               std::vector<EnginePersistedComponentV0>& engineComponents)
{
    serializer.enableBitPacking([&](typename S::BPEnabledType& sbp) {
        sbp.container(
            engineComponents,
            boost::mp11::mp_size<EnginePersistedComponentTypesV0>::value,
            [](typename S::BPEnabledType& serializer,
               EnginePersistedComponentV0& component) {
                serializer.ext(component, bitsery::ext::StdVariant{});
            });
    });
}

namespace DM
{
MigrationStatus MigrateEngineComponents::migrate(SQLite::Database& database,
                                                 unsigned int currentVersion,
                                                 unsigned int desiredVersion)
{
    try {
        for (unsigned int i{currentVersion}; i < desiredVersion; ++i) {
            switch (i) {
                case 0: {
                    migrateV0ToV1(database);
                    break;
                }
                default: {
                    return MigrationStatus::ImplementationMissing;
                }
            }
        }
    } catch (SQLite::Exception& e) {
        LOG_INFO("Database error: %s", e.what());
        return MigrationStatus::DatabaseError;
    }

    return MigrationStatus::Success;
}

void MigrateEngineComponents::migrateV0ToV1(SQLite::Database& database)
{
    // Changes from V0 -> V1:
    //   EngineComponents
    //     Added MyNewComponent

    // Scratch buffers.
    std::vector<EnginePersistedComponentV0> engineComponentsV0{};
    std::vector<Server::EnginePersistedComponent> engineComponents{};
    std::vector<Uint8> workBuffer{};

    // Queries.
    SQLite::Statement iterateEntitiesQuery{database, "SELECT * FROM entities"};
    SQLite::Statement updateEntityQuery{
        database, "UPDATE entities SET engineComponents=? WHERE id=?"};
    SQLite::Statement updateVersionQuery{
        database,
        "UPDATE versions SET versionNumber=? WHERE name='EngineComponents'"};

    // Iterate all entities, updating their engine components.
    while (iterateEntitiesQuery.executeStep()) {
        SQLite::Column idColumn{iterateEntitiesQuery.getColumn(0)};
        SQLite::Column engineComponentDataColumn{
            iterateEntitiesQuery.getColumn(1)};

        // Deserialize the entity's component data as V0.
        engineComponentsV0.clear();
        Deserialize::fromBuffer(
            static_cast<const Uint8*>(engineComponentDataColumn.getBlob()),
            static_cast<std::size_t>(engineComponentDataColumn.getBytes()),
            engineComponentsV0);

        // Copy the data to the new format.
        engineComponents.clear();
        for (auto& engineComponentV0 : engineComponentsV0) {
            // V1 has all of V0's components, so we don't need to do anything 
            // special. Just get it in the new variant format.
            std::visit(VariantTools::Overload(
                [&](const auto& component) {
                    engineComponents.push_back(component);
                }),
                engineComponentV0);
        }

        // Serialize the updated data.
        workBuffer.clear();
        workBuffer.resize(Serialize::measureSize(engineComponents));
        Serialize::toBuffer(workBuffer.data(), workBuffer.size(),
                            engineComponents);

        // Update the database.
        updateEntityQuery.bind(1, workBuffer.data(),
                               static_cast<int>(workBuffer.size()));
        updateEntityQuery.bind(2, idColumn.getInt());
        updateEntityQuery.exec();
        updateEntityQuery.reset();
    }

    // Update successful, set the new version number.
    int newVersionNumber{1};
    updateVersionQuery.bind(1, newVersionNumber);
    updateVersionQuery.exec();
}

} // End namespace DM
} // End namespace AM
