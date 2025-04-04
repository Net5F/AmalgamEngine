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
 * Sent by the server when a nearby entity's components are updated.
 */
struct ComponentUpdate {
    // The EngineMessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::ComponentUpdate};

    /** The tick that this update corresponds to. */
    Uint32 tickNum{0};

    /** The entity's ID. */
    entt::entity entity{entt::null};

    /** The entity's constructed or updated components (we treat them the 
        same). */
    std::vector<ReplicatedComponent> updatedComponents{};

    /** The indices (from ReplicatedComponentTypes) of any of the entity's 
        components that were destroyed. */
    std::vector<Uint8> destroyedComponents{};
};

template<typename S>
void serialize(S& serializer, ComponentUpdate& componentUpdate)
{
    serializer.value4b(componentUpdate.tickNum);
    serializer.value4b(componentUpdate.entity);
    serializer.enableBitPacking([&](typename S::BPEnabledType& sbp) {
        sbp.container(componentUpdate.updatedComponents,
                      boost::mp11::mp_size<ReplicatedComponentTypes>::value,
                      [](typename S::BPEnabledType& serializer,
                         ReplicatedComponent& component) {
                          // Note: This calls serialize() for each type.
                          serializer.ext(component, bitsery::ext::StdVariant{});
                      });
    });
    serializer.container1b(
        componentUpdate.destroyedComponents,
        boost::mp11::mp_size<ReplicatedComponentTypes>::value);
}

} // End namespace AM
