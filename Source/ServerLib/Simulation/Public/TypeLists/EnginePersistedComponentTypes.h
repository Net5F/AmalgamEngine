#pragma once

#include "SaveTimestamp.h"
#include "Name.h"
#include "Input.h"
#include "Position.h"
#include "Rotation.h"
#include "GraphicState.h"
#include "CollisionBitSets.h"
#include "Interaction.h"
#include "ItemHandlers.h"
#include "Dialogue.h"
#include "EntityInitScript.h"
#include "StoredValues.h"
#include "CastCooldown.h"
#include "boost/mp11/list.hpp"
#include "bitsery/traits/vector.h"
#include "bitsery/ext/std_variant.h"
#include <variant>

namespace AM
{
// Note: Persisted components are server-only.
namespace Server
{
/**
 * The version number of the engine's components and component list.
 *
 * If EnginePersistedComponentTypes is changed in any way, or the fields of any
 * component in the list are changed in a way that changes their serialization, 
 * you must increment this number and run a migration.
 */
static constexpr unsigned int ENGINE_COMPONENTS_VERSION{0};

/**
 * All of the engine's component types that should be saved to the database 
 * and loaded at startup.
 *
 * Note: If you change this list in any way, or change the fields of any included
 *       types in a way that breaks serialization, you must increment 
 *       ENGINE_COMPONENTS_VERSION and run a migration.
 * Note: Input implies PreviousPosition, Movement, Rotation, and 
 *       MovementModifiers (movement components).
 *       GraphicState implies Rotation, Collision, and CollisionBitSets 
 *       (graphics components).
 */
using EnginePersistedComponentTypes
    = boost::mp11::mp_list<SaveTimestamp, Name, Input, Position, Rotation,
                           GraphicState, CollisionBitSets, Interaction,
                           ItemHandlers, Dialogue, EntityInitScript,
                           StoredValues, CastCooldown>;

/**
 * A variant that holds a persisted engine component.
 *
 * Used by the server to save entity state to the database.
 */
using EnginePersistedComponent
    = boost::mp11::mp_rename<EnginePersistedComponentTypes, std::variant>;

template<typename S>
void serialize(S& serializer,
               std::vector<EnginePersistedComponent>& engineComponents)
{
    serializer.enableBitPacking([&](typename S::BPEnabledType& sbp) {
        sbp.container(
            engineComponents,
            boost::mp11::mp_size<EnginePersistedComponentTypes>::value,
            [](typename S::BPEnabledType& serializer,
               EnginePersistedComponent& component) {
                serializer.ext(component, bitsery::ext::StdVariant{});
            });
    });
}

} // End namespace Server
} // End namespace AM
