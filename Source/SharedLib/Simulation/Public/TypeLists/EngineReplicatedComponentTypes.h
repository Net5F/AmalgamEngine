#pragma once

#include "IsClientEntity.h"
#include "Name.h"
#include "Input.h"
#include "Movement.h"
#include "MovementModifiers.h"
#include "Rotation.h"
#include "GraphicState.h"
#include "Interaction.h"
#include "boost/mp11/list.hpp"

namespace AM
{

/**
 * A list of the components that the server sends to the client.
 *
 * When a client comes in range of an entity, an init message will be sent that
 * includes the components in this list (if the entity possesses any of them).
 * All client-relevant components must be added to this list, since there's 
 * no other way to send a component during this "init" event.
 * 
 * If you want a component to additionally be sent whenever it's patch()'d, 
 * replace()'d, and destroy()'d, add it to EngineObservedComponentTypes. 
 * If you do choose to observe a component, it still must be added to this list.
 * We enforce this because it always makes sense to init a component if you're 
 * going to observe it.
 * Alternatively, you can handle updates with your own system/messages to gain 
 * some performance (we do this for intense components like Movement and 
 * Rotation).
 *
 * Note: We handle Position separately because every entity is guaranteed to
 *       have one and it's more efficient to do so.
 * Note: We need Rotation, otherwise entities will face the wrong way on init.
 */
using EngineReplicatedComponentTypes
    = boost::mp11::mp_list<IsClientEntity, Name, Input, Movement,
                           MovementModifiers, Rotation, GraphicState,
                           Interaction>;

} // End namespace AM
