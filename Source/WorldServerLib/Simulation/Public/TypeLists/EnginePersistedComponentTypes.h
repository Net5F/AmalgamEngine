#pragma once

#include "PersistedComponentList.h"
#include "SaveTimestamp.h"
#include "Name.h"
#include "Input.h"
#include "Position.h"
#include "Rotation.h"
#include "GraphicState.h"
#include "CollisionBitSets.h"
#include "Interaction.h"
#include "ItemHandler.h"
#include "Dialogue.h"
#include "EntityInitScript.h"
#include "StoredValues.h"
#include "CastCooldown.h"
#include "StableID.h"

namespace AM
{
// Note: Persisted components are server-only.
namespace WorldServer
{
// clang-format off
/**
 * All of the engine's component types that should be saved to the database
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
 * Never re-use IDs. If you add a component, add it to the bottom of the list
 * using the next incremental ID.
 * To remove a component, comment it out instead of deleting it. This leaves
 * documentation to "tombstone" the deprecated ID.
 *
 * If you change a component in any way that causes it to be serialized
 * differently (e.g. adding/removing/changing fields), you must increment the
 * component's VersionNumber and add a migration step to
 * EngineComponentMigrationFunctions.h.
 *
 * Note: Input implies PreviousPosition, Movement, Rotation, and
 *       MovementModifiers (movement components).
 *       GraphicState implies Rotation, Collision, and CollisionBitSets
 *       (graphics components).
 *
 * Note: Each world's tick count starts at 0 when it launches, so a saved tick
 *       is only meaningful relative to other ticks from the same save (e.g.
 *       SaveTimestamp::lastSavedTick). Never compare one to the current tick.
 */
using EnginePersistedComponentTypes = PersistedComponentList<
    PersistedComponentEntry<SaveTimestamp, 1, 0, ClientPersistence::Account>,
    PersistedComponentEntry<Name, 2, 0, ClientPersistence::None>,
    PersistedComponentEntry<Input, 3, 0, ClientPersistence::None>,
    PersistedComponentEntry<Position, 4, 0, ClientPersistence::Account>,
    PersistedComponentEntry<Rotation, 5, 0, ClientPersistence::Account>,
    PersistedComponentEntry<GraphicState, 6, 0, ClientPersistence::Account>,
    PersistedComponentEntry<CollisionBitSets, 7, 0, ClientPersistence::None>,
    PersistedComponentEntry<Interaction, 8, 0, ClientPersistence::None>,
    PersistedComponentEntry<ItemHandler, 9, 0, ClientPersistence::None>,
    PersistedComponentEntry<Dialogue, 10, 0, ClientPersistence::None>,
    PersistedComponentEntry<EntityInitScript, 11, 0, ClientPersistence::None>,
    PersistedComponentEntry<StoredValues, 12, 0, ClientPersistence::World>,
    PersistedComponentEntry<CastCooldown, 13, 0, ClientPersistence::Account>,
    PersistedComponentEntry<StableID, 14, 0, ClientPersistence::None>>;
// clang-format on

} // End namespace WorldServer
} // End namespace AM
