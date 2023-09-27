#pragma once

#include "EngineMessageType.h"
#include "Position.h"
#include "ReplicatedComponent.h"
#include "InitScriptResponse.h"
#include "NetworkDefs.h"
#include "entt/fwd.hpp"
#include "entt/entity/entity.hpp"
#include "bitsery/ext/std_variant.h"
#include "boost/mp11/algorithm.hpp"
#include <SDL_stdinc.h>

namespace AM
{
/**
 * Sent by the client to request that an entity be created, or to 
 * request that an existing entity be re-initialized.
 */
struct EntityInitRequest {
    // The EngineMessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::EntityInitRequest};

    //--------------------------------------------------------------------------
    // Networked data
    //--------------------------------------------------------------------------
    /** If non-null, this message is a request to re-init this entity. If null,
        this message is a request to create a new entity. */
    entt::entity entity{entt::null};

    /** The entity's Position. */
    Position position{};

    /** The entity's optional client-relevant components.
        We use this for components that build mode will want to set directly. */
    std::vector<ReplicatedComponent> components{};

    /** The script to run on this entity after creation. */
    std::string initScript{};

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
void serialize(S& serializer,
               EntityInitRequest& entityInitRequest)
{
    serializer.value4b(entityInitRequest.entity);
    serializer.enableBitPacking([&](typename S::BPEnabledType& sbp) {
        sbp.container(entityInitRequest.components,
                      boost::mp11::mp_size<ReplicatedComponentTypes>::value,
                      [](typename S::BPEnabledType& serializer,
                         ReplicatedComponent& component) {
                          serializer.ext(component, bitsery::ext::StdVariant{});
                      });
    });
    serializer.text1b(entityInitRequest.initScript,
                      InitScriptResponse::MAX_SCRIPT_LENGTH);
}

} // End namespace AM
