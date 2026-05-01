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
 * Note that each list has an engine and project version, e.g. 
 * EngineInRangeInitComponentTypes.h, ProjectInRangeInitComponentTypes.h.
 *
 * ## In-Range Init Components ##
 * When a client comes in range of an entity, an init message will be sent that
 * includes the components in this list (if the entity possesses any of them).
 * Even if you send updates manually, you may want to add your component to 
 * this list, since there's no other way to send a component during this "in-
 * range init" event.
 * Additionally, handles sending an init message to the owning entity's client 
 * when it enters the world (since the owning entity is in-range of itself).
 *
 * ## Observe-Broadcast Components ##
 * When registry.patch()/replace()/destroy() are used to update a component in 
 * this list, the new component state will be broadcast to all nearby clients.
 * If you choose to observe-broadcast a component, it still must be added to the 
 * In-Range Init list. We enforce this because it always makes sense to init a 
 * component if you're going to observe it.
 * The performance overhead (compared to sending custom messages manually) is 
 * small (~10-15%), but for intense components, this may not be acceptable. Or, 
 * you may have a large component where you only want to send changed data 
 * instead of the full component each time. In either case, you should create 
 * a custom update message and send it manually in your own system.
 *
 * ## Observe-Self Components ##
 * When registry.patch()/replace()/destroy() are used to update a component in
 * this list, the new component state will be sent to the owning entity's 
 * client.
 * Additionally, handles sending the component to the client when it's first 
 * created on the server (e.g. when entering the world). Thus, there's no need
 * to also add the component to the In-Range Init list.
 */

using EngineInRangeInitComponentTypes
    = boost::mp11::mp_list<IsClientEntity, Name, Input, Movement,
                           MovementModifiers, Rotation, GraphicState,
                           CollisionBitSets, Interaction>;

using EngineObserveBroadcastComponentTypes
    = boost::mp11::mp_list<Name, MovementModifiers, GraphicState,
                           CollisionBitSets, Interaction>;

using EngineObserveSelfComponentTypes = boost::mp11::mp_list<>;

/**
 * This engine provides convenient lists that you can add components to, to have
 * them be automatically replicated from the server to clients. 
 *
 * The lists are split into 4 types. Each list only acts on components that 
 * belong to client entities:
 *
 * ## Init-Self Components ##
 * When a component in this list is constructed or destroyed, it will be 
 * replicated to the owning client.
 * This includes when the client entity enters the world (login).
 *
 * ## Update-Self Components ##
 * When a component in this list is updated through a registry function (e.g. 
 * patch()), it will be replicated to the owning client.
 *
 * ## Init-All Components ##
 * When a component in this list is constructed or destroyed, it will be 
 * replicated to the owning client and all nearby clients.
 * This includes when the client entity enters the world (login).
 *
 * Even if you send updates manually, you may want to add your component to 
 * this list, since there's no other way to send a component during this "in-
 * range init" event.
 *
 * ## Update-All Components ##
 * When a component in this list is updated through a registry function (e.g. 
 * patch()), it will be replicated to the owning client and all nearby clients.
 *
 * ## Additional Notes ##
 * - Every component in one of the Update lists must also be in the 
 *   corresponding Init list. We enforce this because it always makes sense to 
 *   init and component if you're going to update it.
 * - For performance-intensive components, it's valid to add them to the Init 
 *   list, but handle updates manually (with custom messages, sent from your 
 *   own system). The performance overhead of the Update process is 10-15%.
 * - Each list has an engine and project version, e.g. 
 *   EngineInitSelfComponentTypes.h, ProjectInitSelfComponentTypes.h.
 */

using EngineInitSelfComponentTypes = boost::mp11::mp_list<>;

using EngineUpdateSelfComponentTypes = boost::mp11::mp_list<>;

/**
 * Note: We handle Position separately because every entity is guaranteed to
 *       have one and it's more efficient to do so.
 * Note: We need Rotation, otherwise entities will face the wrong way on init.
 */
using EngineInitAllComponentTypes
    = boost::mp11::mp_list<IsClientEntity, Name, Input, Movement,
                           MovementModifiers, Rotation, GraphicState,
                           CollisionBitSets, Interaction>;

// TODO: All vs InRange?
using EngineUpdateAllComponentTypes
    = boost::mp11::mp_list<Name, MovementModifiers, GraphicState,
                           CollisionBitSets, Interaction>;

} // End namespace AM
