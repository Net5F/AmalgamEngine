#pragma once

#include "IsClientEntity.h"
#include "Name.h"
#include "Input.h"
#include "Movement.h"
#include "MovementModifiers.h"
#include "Rotation.h"
#include "GraphicState.h"
#include "CollisionBitSets.h"
#include "Interaction.h"
#include "boost/mp11/list.hpp"

namespace AM
{

/**
 * This engine provides convenient lists that you can add components to, to have
 * them be automatically replicated from the server to clients. 
 *
 * The lists are split into 4 types. Each list only acts on components that 
 * belong to client entities:
 *
 * ## Self-Init Components ##
 * When a component in this list is constructed or destroyed, it will be 
 * replicated to the owning client.
 * This includes when the client entity enters the world (login).
 *
 * ## Self-Update Components ##
 * When a component in this list is updated through a registry function (e.g. 
 * patch()), it will be replicated to the owning client.
 *
 * ## In-Range-Init Components ##
 * When a component in this list is constructed or destroyed, it will be 
 * replicated to the owning client and all clients within 
 * SharedConfig::AOI_RADIUS of the client entity.
 * This includes when the client entity enters the world (login).
 *
 * Even if you send updates manually, you may want to add your component to 
 * this list, since there's no other way to send a component during this "in-
 * range init" event.
 *
 * ## In-Range-Update Components ##
 * When a component in this list is updated through a registry function (e.g. 
 * patch()), it will be replicated to the owning client and all clients within 
 * SharedConfig::AOI_RADIUS of the client entity.
 *
 * ## Additional Notes ##
 * - Every component in one of the Update lists must also be in the 
 *   corresponding Init list. We enforce this because it always makes sense to 
 *   init a component if you're going to update it.
 * - For performance-intensive components, it's valid to add them to the Init 
 *   list, but handle updates manually (with custom messages, sent from your 
 *   own system). The performance overhead of the Update process is 10-15%.
 * - Each list has an engine and project version, e.g. 
 *   EngineInitSelfComponentTypes.h, ProjectInitSelfComponentTypes.h.
 */

using EngineSelfInitComponentTypes = boost::mp11::mp_list<>;

using EngineSelfUpdateComponentTypes = boost::mp11::mp_list<>;

/**
 * Note: We handle Position separately because every entity is guaranteed to
 *       have one and it's more efficient to do so.
 * Note: We need Rotation, otherwise entities will face the wrong way on init.
 */
using EngineInRangeInitComponentTypes
    = boost::mp11::mp_list<IsClientEntity, Name, Input, Movement,
                           MovementModifiers, Rotation, GraphicState,
                           CollisionBitSets, Interaction>;

using EngineInRangeUpdateComponentTypes
    = boost::mp11::mp_list<Name, MovementModifiers, GraphicState,
                           CollisionBitSets, Interaction>;

} // End namespace AM
