#pragma once

#include "EngineMessageType.h"
#include "entt/fwd.hpp"
#include "entt/entity/entity.hpp"

namespace AM
{
/**
 * Used to request that an entity be deleted, or to inform a client that an
 * entity was deleted.
 */
struct EntityDelete {
    // The EngineMessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::EntityDelete};

    /** The tick that this update corresponds to. */
    Uint32 tickNum{0};

    /** The entity that must be deleted. */
    entt::entity entity{entt::null};
};

template<typename S>
void serialize(S& serializer, EntityDelete& entityDelete)
{
    serializer.value4b(entityDelete.tickNum);
    serializer.value4b(entityDelete.entity);
}

} // End namespace AM
