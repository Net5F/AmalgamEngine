#pragma once

#include "MessageType.h"
#include "entt/fwd.hpp"
#include "entt/entity/entity.hpp"
#include <string>

namespace AM
{
/**
 * Sent by the server to tell a client that an entity has entered its area
 * of interest and must be constructed.
 *
 * Contains the entity's "static data"--that is, the data that doesn't change
 * often (such as the entity's name).
 */
struct EntityInit {
    // The MessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr MessageType MESSAGE_TYPE{MessageType::EntityInit};

    /** Used as a "we should never hit this" cap on the length of the name
        string. Only checked in debug builds. */
    static constexpr std::size_t NAME_LENGTH{50};

    /** The tick that this update corresponds to. */
    Uint32 tickNum{0};

    /** The entity that must be constructed. */
    entt::entity entity{entt::null};

    /** The entity's name. */
    std::string name{"Empty"};

    /** The numeric identifier for the entity's sprite. */
    int numericID{-1};
};

template<typename S>
void serialize(S& serializer, EntityInit& entityInit)
{
    serializer.value4b(entityInit.tickNum);
    serializer.value4b(entityInit.entity);
    serializer.text1b(entityInit.name, EntityInit::NAME_LENGTH);
    serializer.value4b(entityInit.numericID);
}

} // End namespace AM
