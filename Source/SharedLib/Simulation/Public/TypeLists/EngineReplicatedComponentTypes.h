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
 * See ComponentTypeRegistry.h for more info.
 *
 * Note: We handle Position separately because every entity is guaranteed to
 *       have one and it's more efficient to do so.
 * Note: We need Rotation, otherwise entities will face the wrong way on init.
 */
using EngineReplicatedComponentTypes
    = boost::mp11::mp_list<IsClientEntity, Name, Input, Rotation,
                           AnimationState, Interaction>;

} // End namespace AM
