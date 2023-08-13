#pragma once

#include "EngineMessageType.h"
#include "Position.h"
#include "PreviousPosition.h"
#include "Velocity.h"
#include "Rotation.h"
#include "entt/fwd.hpp"
#include "entt/entity/entity.hpp"
#include <string>

namespace AM
{
/**
 * Sent by the server to tell a client when a client entity enters their area 
 * of interest.
 *
 * We handle client entities separately from non-client for two reasons:
 *   1. We already know what components they need, so we can save data.
 *   2. It's useful to know which entities belong to clients, because they 
 *      aren't modifiable in build mode.
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

    /** The client entity's entity ID. */
    entt::entity entity{entt::null};

    /** The entity's name. */
    std::string name{"Empty"};

    /** The entity's world position. */
    Position position{};

    /** The entity's rotation. */
    Rotation rotation{};

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
    serializer.object(clientEntityInit.position);
    serializer.object(clientEntityInit.rotation);
    serializer.value4b(clientEntityInit.numericID);
}

} // End namespace AM
