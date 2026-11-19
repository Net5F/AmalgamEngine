#pragma once

#include "EngineMessageType.h"
#include "ModerationDefs.h"
#include "AccountDefs.h"
#include "ConnectionHandle.h"
#include <SDL3/SDL_stdinc.h>
#include <string>

namespace AM
{

/**
 * Sent by a client to take a world-level moderation action, e.g. teleporting
 * a player.
 *
 * The server checks the requester's role (see AccountRole), applies the
 * action, and responds with a WorldModerationResponse. These actions only
 * affect this world, so AccountServer isn't involved.
 *
 * For account-level actions (bans, roles), see AccountModerationRequest.
 */
struct WorldModerationRequest {
    // The MessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::WorldModerationRequest};

    //--------------------------------------------------------------------------
    // Networked data
    //--------------------------------------------------------------------------
    WorldModerationAction action{WorldModerationAction::Teleport};

    /** The username of the player to act on. */
    std::string targetUsername{};

    // TODO: Add the teleport destination.

    //--------------------------------------------------------------------------
    // Local data
    //--------------------------------------------------------------------------
    /** The handle of the client that sent this message.
        Set by the server, based on which connection the message came
        from (we can't trust the client to ID itself). */
    ConnectionHandle clientHandle{};
};

template<typename S>
void serialize(S& serializer, WorldModerationRequest& worldModerationRequest)
{
    serializer.value1b(worldModerationRequest.action);
    serializer.text1b(worldModerationRequest.targetUsername,
                      USERNAME_MAX_LENGTH);
}

} // End namespace AM
