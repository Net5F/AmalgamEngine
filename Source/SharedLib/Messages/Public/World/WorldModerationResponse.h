#pragma once

#include "EngineMessageType.h"
#include "ModerationDefs.h"
#include "AccountDefs.h"
#include <SDL3/SDL_stdinc.h>
#include <string>

namespace AM
{

/**
 * Sent by the server in response to a WorldModerationRequest.
 */
struct WorldModerationResponse {
    // The MessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::WorldModerationResponse};

    /** The action that was requested. */
    WorldModerationAction action{WorldModerationAction::Teleport};

    ModerationResult result{ModerationResult::InternalError};

    /** The target's username, as given in the request. */
    std::string targetUsername{};
};

template<typename S>
void serialize(S& serializer, WorldModerationResponse& worldModerationResponse)
{
    serializer.value1b(worldModerationResponse.action);
    serializer.value1b(worldModerationResponse.result);
    serializer.text1b(worldModerationResponse.targetUsername,
                      USERNAME_MAX_LENGTH);
}

} // End namespace AM
