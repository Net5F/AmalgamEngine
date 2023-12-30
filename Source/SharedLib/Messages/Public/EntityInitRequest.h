#pragma once

#include "EngineMessageType.h"
#include "Name.h"
#include "Position.h"
#include "AnimationState.h"
#include "EntityInitScript.h"
#include "NetworkDefs.h"
#include "entt/fwd.hpp"
#include "entt/entity/entity.hpp"

namespace AM
{
/**
 * Sent by the client to request that a new entity be created, or to
 * request that an existing entity be re-initialized with new data.
 */
struct EntityInitRequest {
    // The EngineMessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::EntityInitRequest};

    //--------------------------------------------------------------------------
    // Networked data
    //--------------------------------------------------------------------------
    /** If non-null, this message is a request to re-init this entity. If null,
        this message is a request to create a new entity. */
    entt::entity entity{entt::null};

    /** The entity's components. */
    Name name{};
    Position position{};
    AnimationState animationState{};

    /** The script to run on this entity after creation. */
    EntityInitScript initScript{};

    //--------------------------------------------------------------------------
    // Local data
    //--------------------------------------------------------------------------
    /**
     * The network ID of the client that sent this message.
     * Set by the server.
     * No IDs are accepted from the client because we can't trust it,
     * so we fill in the ID based on which socket the message came from.
     */
    NetworkID netID{0};
};

template<typename S>
void serialize(S& serializer, EntityInitRequest& entityInitRequest)
{
    serializer.value4b(entityInitRequest.entity);
    serializer.object(entityInitRequest.name);
    serializer.object(entityInitRequest.position);
    serializer.object(entityInitRequest.animationState);
    serializer.object(entityInitRequest.initScript);
}

} // End namespace AM
