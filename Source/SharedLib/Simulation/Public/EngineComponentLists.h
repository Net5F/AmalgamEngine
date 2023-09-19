#pragma once

#include "IsClientEntity.h"
#include "Name.h"
#include "Input.h"
#include "Position.h"
#include "Velocity.h"
#include "Rotation.h"
#include "AnimationState.h"
#include "Interaction.h"
#include "boost/mp11/list.hpp"
#include "boost/mp11/map.hpp"

namespace AM
{
namespace EngineComponentLists
{

/**
 * All of the project's component types that are relevant to the client.
 *
 * When a client comes in range of an entity, an Init message that includes these
 * components will be sent (if the entity possesses any of them).
 *
 * In other words, adding components to this list will cause them to be sent 
 * once. If you want a component to additionally be sent whenever it's updated, 
 * add it to ObservedComponentMap below.
 */
using ReplicatedComponentTypes
    = boost::mp11::mp_list<IsClientEntity, Name, Input, Position, Velocity,
                           Rotation, AnimationState, Interaction>;

/**
 * A map of "ObservedComponent" -> "SendComponents".
 * The first type in each list is the ObservedComponent, the rest are the 
 * SendComponents.
 *
 * When an ObservedComponent is updated (using e.g. patch()), an Update message
 * containing the SendComponents will be sent by the server to all nearby 
 * clients.
 *
 * Note: All of the SendComponents must be in ReplicatedComponentTypes.
 *       The ObservedComponents don't need to be in ReplicatedComponentTypes.
 */
using ObservedComponentMap = boost::mp11::mp_list<
    boost::mp11::mp_list<IsClientEntity, IsClientEntity>,
    //boost::mp11::mp_list<Input, Input, Position, Velocity, Rotation>,
    boost::mp11::mp_list<Name, Name>,
    boost::mp11::mp_list<AnimationState, AnimationState>,
    boost::mp11::mp_list<Interaction, Interaction>>;

} // End namespace EngineComponentLists
} // End namespace AM
