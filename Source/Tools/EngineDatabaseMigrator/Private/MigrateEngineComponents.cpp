#include "MigrateEngineComponents.h"
#include "EnginePersistedComponentTypes.h"
#include "SQLiteCpp/SQLiteCpp.h"
#include "Log.h"

namespace AM
{
namespace DM
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

MigrationStatus MigrateEngineComponents::migrate(SQLite::Database& database,
                                                 unsigned int currentVersion,
                                                 unsigned int desiredVersion)
{
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

    return MigrationStatus::Success;
}

void MigrateEngineComponents::migrateV0ToV1(SQLite::Database& database)
{
    LOG_INFO("migrateV0ToV1()");
    // TODO: Check that db fails to load in server
    // TODO: Iterate EngineComponents rows. Deserialize as V0, move into 
    //       current, reserialize and update row.
    //       If all good, update version
}

} // End namespace DM
} // End namespace AM
