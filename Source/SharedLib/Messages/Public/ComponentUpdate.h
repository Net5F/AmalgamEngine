#pragma once

#include "EngineMessageType.h"
#include "SerializedComponent.h"
#include "entt/fwd.hpp"
#include "entt/entity/entity.hpp"
#include <SDL_stdinc.h>

namespace AM
{
/**
 * Sent by the server when a nearby entity's components are updated.
 */
struct ComponentUpdate {
    // The EngineMessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::ComponentUpdate};

    /** Used as a "we should never hit this" cap on the number of replicated
        components for this entity. Only checked in debug builds. */
    static constexpr std::size_t MAX_COMPONENTS{100};

    /** The tick that this update corresponds to. */
    Uint32 tickNum{0};

    /** The entity's ID. */
    entt::entity entity{entt::null};

    /** The entity's updated components. */
    std::vector<SerializedComponent> components{};
};

template<typename S>
void serialize(S& serializer, ComponentUpdate& componentUpdate)
{
    serializer.value4b(componentUpdate.tickNum);
    serializer.value4b(componentUpdate.entity);
    serializer.container(componentUpdate.components,
                         ComponentUpdate::MAX_COMPONENTS);
}

} // End namespace AM
