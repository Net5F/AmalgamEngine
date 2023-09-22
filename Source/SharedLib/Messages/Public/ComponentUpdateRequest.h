#pragma once

#include "EngineMessageType.h"
#include "ReplicatedComponent.h"
#include "NetworkDefs.h"
#include "entt/fwd.hpp"
#include "entt/entity/entity.hpp"
#include "bitsery/ext/std_variant.h"
#include "boost/mp11/algorithm.hpp"

namespace AM
{
/**
 * Sent by the client to request that an entity's components be updated. 
 * 
 * Note: We restrict the component types to those in ReplicatedComponent
 *       only because it has all that we currently need and it's convenient.
 *       If you need more than this, you can use EntityInitRequest to re-init.
 *       We can also switch to a new list e.g. EditableComponentTypes, but 
 *       this message isn't likely to be used much (maybe only by build mode).
 *       Most component changes will be done by a system responding to a more 
 *       specific message, the client won't request the change directly.
 */
struct ComponentUpdateRequest {
    // The EngineMessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::ComponentUpdateRequest};

    //--------------------------------------------------------------------------
    // Networked data
    //--------------------------------------------------------------------------
    /** The entity's ID. */
    entt::entity entity{entt::null};

    /** The entity's updated components. */
    std::vector<ReplicatedComponent> components{};

    //--------------------------------------------------------------------------
    // Local data
    //--------------------------------------------------------------------------
    /**
     * The network ID of the client that sent this message.
     * Set by the server.
     * No IDs are accepted from the client because we can't trust it,
     * so we fill in the ID based on which socket the message came from.
     */
    NetworkID netID{0};
};

template<typename S>
void serialize(S& serializer, ComponentUpdateRequest& componentUpdateRequest)
{
    serializer.value4b(componentUpdateRequest.entity);
    serializer.enableBitPacking([&](typename S::BPEnabledType& sbp) {
        sbp.container(componentUpdateRequest.components,
                      boost::mp11::mp_size<ReplicatedComponentTypes>::value,
                      [](typename S::BPEnabledType& serializer,
                         ReplicatedComponent& component) {
                          serializer.ext(component, bitsery::ext::StdVariant{});
                      });
    });
}

} // End namespace AM
