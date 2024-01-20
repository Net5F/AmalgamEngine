#pragma once

#include "IsClientEntity.h"
#include "Name.h"
#include "Input.h"
#include "Rotation.h"
#include "AnimationState.h"
#include "Interaction.h"
#include "boost/mp11/list.hpp"

namespace AM
{

/**
 * All of the engine's component types that are relevant to the client.
 *
 * When a client comes in range of an entity, an Init message that includes
 * these components will be sent (if the entity possesses any of them).
 *
 * In other words, adding components to this list will cause them to be sent
 * once. If you want a component to additionally be sent whenever it's updated,
 * add it to EngineObservedComponentTypes.
 *
 * Note: We handle Position separately because every entity is guaranteed to
 *       have one and it's more efficient to do so.
 * Note: We need Rotation, otherwise entities will face the wrong way on init.
 */
using EngineReplicatedComponentTypes
    = boost::mp11::mp_list<IsClientEntity, Name, Input, Rotation,
                           AnimationState, Interaction>;

} // End namespace AM
