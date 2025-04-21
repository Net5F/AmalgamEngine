#pragma once

#include "ItemInteractionType.h"
#include "EntityInteractionType.h"
#include "SpellType.h"
#include "NetworkID.h"
#include "entt/fwd.hpp"
#include <SDL_stdinc.h>

namespace AM
{
struct Vector3;

/**
 * This file contains structs meant to be used to pass parameters into 
 * Client::CastHelper and Server::CastHelper functions.
 */

struct CastItemInteractionParams {
    /** The item interaction to cast. */
    ItemInteractionType interactionType{};
    /** The entity that is casting. */
    entt::entity casterEntity{};
    /** The slot of the item that is being used, within casterEntity's 
        inventory. */
    Uint8 slotIndex{};
    /** (Optional) The target entity. If the Castable's targetToolType is 
        Entity, this must be valid. */
    entt::entity targetEntity{};
    /** (Optional) The target position. If the Castable's targetToolType 
        is Circle, this must be valid. */
    const Vector3& targetPosition{};
    /** (Optional) The client that requested this cast. If casterEntity is a 
        client entity, you must fill this in. Otherwise it won't be replicated 
        properly. */
    NetworkID clientID{NULL_NETWORK_ID};
};

struct CastEntityInteractionParams {
    /** The entity interaction to cast. */
    EntityInteractionType interactionType{};
    /** The entity that is casting. */
    entt::entity casterEntity{};
    /** The target entity. */
    entt::entity targetEntity{};
    /** (Optional) The target position. If the Castable's targetToolType 
        is Circle, this must be valid. */
    const Vector3& targetPosition{};
    /** (Optional) The client that requested this cast. If casterEntity is a 
        client entity, you must fill this in. Otherwise it won't be replicated 
        properly. */
    NetworkID clientID{NULL_NETWORK_ID};
};

struct CastSpellParams {
    /** The item interaction to cast. */
    SpellType interactionType{};
    /** The entity that is casting. */
    entt::entity casterEntity{};
    /** (Optional) The target entity. If the Castable's targetToolType is 
        Entity, this must be valid. */
    entt::entity targetEntity{};
    /** (Optional) The client that requested this cast. If present, any
        failure messages will be sent to this client. */
    const Vector3& targetPosition{};
    /** (Optional) The client that requested this cast. If casterEntity is a 
        client entity, you must fill this in. Otherwise it won't be replicated 
        properly. */
    NetworkID clientID{NULL_NETWORK_ID};
};

} // End namespace AM
