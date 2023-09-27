#pragma once

#include "EngineMessageType.h"
#include "Position.h"
#include "ReplicatedComponent.h"
#include "SharedConfig.h"
#include "entt/fwd.hpp"
#include "entt/entity/entity.hpp"
#include "bitsery/ext/std_variant.h"
#include "boost/mp11/algorithm.hpp"
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
        /** The entity's ID. */
        entt::entity entity{entt::null};

        /** The entity's Position. */
        Position position{};

        /** This entity's optional client-relevant components. */
        std::vector<ReplicatedComponent> components{};
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
    serializer.enableBitPacking([&](typename S::BPEnabledType& sbp) {
        sbp.container(entityData.components,
                      boost::mp11::mp_size<ReplicatedComponentTypes>::value,
                      [](typename S::BPEnabledType& serializer,
                         ReplicatedComponent& component) {
                          serializer.ext(component, bitsery::ext::StdVariant{});
                      });
    });
}

template<typename S>
void serialize(S& serializer, EntityInit& entityInit)
{
    serializer.value4b(entityInit.tickNum);
    serializer.container(entityInit.entityData, SharedConfig::MAX_ENTITIES);
}

} // End namespace AM
