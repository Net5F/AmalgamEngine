#pragma once

#include "PersistedComponentList.h"

namespace AM
{
// Note: Persisted components are server-only.
namespace WorldServer
{
// clang-format off
/**
 * All of the project's component types that should be saved to the database
 * and loaded at startup.
 *
 * Format: <ComponentType, ID, VersionNumber, ClientPersistence>
 *
 * ClientPersistence controls where the component is saved when it's
 * attached to a client entity (see PersistedComponentList.h). Account data
 * follows the player to every world, World data stays on this world, and
 * None data is rebuilt when the entity spawns.
 *
 * Rules:
 * Never reuse IDs. If you add a component, add it to the bottom of the list
 * using the next incremental ID.
 * To remove a component, comment it out instead of deleting it. This leaves
 * documentation to "tombstone" the deprecated ID.
 *
 * If you change a component in any way that causes it to be serialized
 * differently (e.g. adding/removing/changing fields), you must increment the
 * component's VersionNumber and add a migration step to
 * ComponentMigrationFunctions.h.
 */
using ProjectPersistedComponentTypes = PersistedComponentList<
    // PersistedComponentEntry<ProjectComponent, 1, 0, ClientPersistence::Account>
>;
// clang-format on

} // End namespace WorldServer
} // End namespace AM

#endif // defined(AM_OVERRIDE_CONFIG)
