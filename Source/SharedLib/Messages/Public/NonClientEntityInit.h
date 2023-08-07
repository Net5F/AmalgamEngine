#pragma once

#include "EngineMessageType.h"
#include "NonClientEntityInit.h"
#include "Position.h"
#include "entt/fwd.hpp"
#include "entt/entity/entity.hpp"
#include <string>

namespace AM
{
/**
 * Sent by the server to inform the client of a client entity (see 
 * IsClientEntity.h for more info).
 *
 * We handle client entities separately from non-client for two reasons:
 *   1. We already know what components they need, so we can save data.
 *   2. It's useful to know which entities belong to clients, because they 
 *      aren't modifiable in build mode.
 *
 * For entities not controlled by a client, see NonClientEntityInit.
 *
 * Contains the entity's "static data"--that is, the data that doesn't change
 * often (such as the entity's name).
 */
struct NonClientEntityInit {
    // The EngineMessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::NonClientEntityInit};

    /** The tick that this update corresponds to. */
    Uint32 tickNum{0};

    /** The entity that must be constructed. */
    entt::entity entity{entt::null};

    /** The entity's world position. Kept separate from the components vector 
        because all entities have a position. */
    Position position{};

    // TODO: Figure out what we're doing with sprite sets.
    /** The numeric identifier for the entity's sprite. */
    int numericID{-1};

    // TODO: Vector?
};

template<typename S>
void serialize(S& serializer, NonClientEntityInit& nonClientEntityInit)
{
    serializer.value4b(nonClientEntityInit.tickNum);
    serializer.value4b(nonClientEntityInit.entity);
    serializer.value4b(nonClientEntityInit.numericID);
    serializer.object(nonClientEntityInit.position);
}

} // End namespace AM
