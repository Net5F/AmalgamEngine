#pragma once

#include "EngineMessageType.h"
#include "NetworkID.h"
#include "entt/fwd.hpp"
#include "entt/entity/entity.hpp"
#include <SDL_stdinc.h>

namespace AM
{

/**
 * Sent by a client to request that a dialogue choice be selected.
 */
struct DialogueChoiceRequest {
    // The MessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::DialogueChoiceRequest};

    //--------------------------------------------------------------------------
    // Networked data
    //--------------------------------------------------------------------------
    /** The entity that the client is talking to. */
    entt::entity targetEntity{entt::null};

    /** The index of the desired topic within the entity's Dialogue::topics. */
    Uint8 topicIndex{0};

    /** The index of the desired choice within the topic's Topic::choices. */
    Uint8 choiceIndex{0};

    //--------------------------------------------------------------------------
    // Local data
    //--------------------------------------------------------------------------
    /** The network ID of the client that sent this message.
        Set by the server.
        No IDs are accepted from the client because we can't trust it, so we
        fill in the ID based on which socket the message came from. */
    NetworkID netID{0};
};

template<typename S>
void serialize(S& serializer, DialogueChoiceRequest& dialogueChoiceRequest)
{
    serializer.value4b(dialogueChoiceRequest.targetEntity);
    serializer.value1b(dialogueChoiceRequest.topicIndex);
    serializer.value1b(dialogueChoiceRequest.choiceIndex);
}

} // End namespace AM
