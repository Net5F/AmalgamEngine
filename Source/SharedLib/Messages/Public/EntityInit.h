#pragma once

#include "EngineMessageType.h"
#include "Position.h"
#include "SerializedComponent.h"
#include "SharedConfig.h"
#include "entt/fwd.hpp"
#include "entt/entity/entity.hpp"
#include <SDL_stdinc.h>
#include <vector>

namespace AM
{

/**
 * Sent by the server when an entity enters a client's area of interest.
 *
 * Contains all entities that entered the client's AOI on a single sim tick.
 *
 * Each client is only sent the state of entities that are in their area of
 * interest.
 */
struct EntityInit {
    // The EngineMessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::EntityInit};

    /** The tick that this update corresponds to. */
    Uint32 tickNum{0};

    struct EntityData {
        /** Used as a "we should never hit this" cap on the number of replicated
            components for this entity. Only checked in debug builds. */
        static constexpr std::size_t MAX_COMPONENTS{100};

        /** The entity's ID. */
        entt::entity entity{entt::null};

        /** The entity's Position. */
        Position position{};

        /** This entity's optional client-relevant components. */
        std::vector<SerializedComponent> components{};
    };

    /** The component state of all entities that entered this client's AOI on
        this tick. */
    std::vector<EntityData> entityData{};
};

template<typename S>
void serialize(S& serializer, EntityInit::EntityData& entityData)
{
    serializer.value4b(entityData.entity);
    serializer.object(entityData.position);
    serializer.container(entityData.components,
                         EntityInit::EntityData::MAX_COMPONENTS);
}

template<typename S>
void serialize(S& serializer, EntityInit& entityInit)
{
    serializer.value4b(entityInit.tickNum);
    serializer.container(entityInit.entityData, SharedConfig::MAX_ENTITIES);
}

} // End namespace AM
