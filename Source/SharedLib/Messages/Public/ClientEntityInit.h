#pragma once

#include "EngineMessageType.h"
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
struct ClientEntityInit {
    // The EngineMessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::ClientEntityInit};

    /** Used as a "we should never hit this" cap on the length of the name
        string. Only checked in debug builds. */
    static constexpr std::size_t NAME_LENGTH{50};

    /** The tick that this update corresponds to. */
    Uint32 tickNum{0};

    /** The entity that must be constructed. */
    entt::entity entity{entt::null};

    /** The entity's name. */
    std::string name{"Empty"};

    /** The entity's world position. */
    Position position{};

    // TODO: Figure out what we're doing with sprite sets.
    /** The numeric identifier for the entity's sprite. */
    int numericID{-1};
};

template<typename S>
void serialize(S& serializer, ClientEntityInit& clientEntityInit)
{
    serializer.value4b(clientEntityInit.tickNum);
    serializer.value4b(clientEntityInit.entity);
    serializer.text1b(clientEntityInit.name, ClientEntityInit::NAME_LENGTH);
    serializer.value4b(clientEntityInit.numericID);
    serializer.object(clientEntityInit.position);
}

} // End namespace AM
