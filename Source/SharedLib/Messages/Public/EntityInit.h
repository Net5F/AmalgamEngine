#pragma once

#include "EngineMessageType.h"
#include "ReplicatedComponent.h"
#include "entt/fwd.hpp"
#include "entt/entity/entity.hpp"
#include "bitsery/ext/std_variant.h"
#include "boost/mp11/algorithm.hpp"

namespace AM
{
/**
 * Sent by the server when an entity enters a client's area of interest. 
 */
struct EntityInit {
    // The EngineMessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::EntityInit};

    /** The tick that this update corresponds to. */
    Uint32 tickNum{0};

    /** The entity's ID. */
    entt::entity entity{entt::null};

    /** This entity's client-relevant components. */
    std::vector<ReplicatedComponent> components{};
};

template<typename S>
void serialize(S& serializer, EntityInit& entityInit)
{
    serializer.value4b(entityInit.tickNum);
    serializer.value4b(entityInit.entity);
    serializer.enableBitPacking([&](typename S::BPEnabledType& sbp) {
        sbp.container(entityInit.components,
                      boost::mp11::mp_size<ReplicatedComponentTypes>::value,
                      [](typename S::BPEnabledType& serializer,
                         ReplicatedComponent& component) {
                          serializer.ext(component, bitsery::ext::StdVariant{});
                      });
    });
}

} // End namespace AM
