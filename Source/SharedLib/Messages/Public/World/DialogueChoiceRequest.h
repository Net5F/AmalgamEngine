#pragma once

#include "EngineMessageType.h"
#include "ConnectionHandle.h"
#include "entt/fwd.hpp"
#include "entt/entity/entity.hpp"
#include <SDL3/SDL_stdinc.h>

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
    /** The handle of the client that sent this message.
        Set by the server, based on which connection the message came
        from (we can't trust the client to ID itself). */
    ConnectionHandle clientHandle{};
};

template<typename S>
void serialize(S& serializer, DialogueChoiceRequest& dialogueChoiceRequest)
{
    serializer.value4b(dialogueChoiceRequest.targetEntity);
    serializer.value1b(dialogueChoiceRequest.topicIndex);
    serializer.value1b(dialogueChoiceRequest.choiceIndex);
}

} // End namespace AM
