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

namespace AM
{
// Note: Persisted components are server-only.
namespace Server
{
// clang-format off
/**
 * All of the engine's component types that should be saved to the database
 * and loaded at startup.
 *
 * Format: <ComponentType, ID, VersionNumber>
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
 */
using EnginePersistedComponentTypes = PersistedComponentList<
    PersistedComponentEntry<SaveTimestamp, 1, 0>,
    PersistedComponentEntry<Name, 2, 0>,
    PersistedComponentEntry<Input, 3, 0>,
    PersistedComponentEntry<Position, 4, 0>,
    PersistedComponentEntry<Rotation, 5, 0>,
    PersistedComponentEntry<GraphicState, 6, 0>,
    PersistedComponentEntry<CollisionBitSets, 7, 0>,
    PersistedComponentEntry<Interaction, 8, 0>,
    PersistedComponentEntry<ItemHandler, 9, 0>,
    PersistedComponentEntry<Dialogue, 10, 0>,
    PersistedComponentEntry<EntityInitScript, 11, 0>,
    PersistedComponentEntry<StoredValues, 12, 0>,
    PersistedComponentEntry<CastCooldown, 13, 0>>;
// clang-format on

} // End namespace Server
} // End namespace AM
