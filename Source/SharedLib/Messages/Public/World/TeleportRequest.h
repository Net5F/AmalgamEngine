#pragma once

#include "EngineMessageType.h"
#include "AccountDefs.h"
#include "ConnectionHandle.h"
#include "TilePosition.h"
#include "bitsery/ext/std_variant.h"
#include <string>
#include <variant>

namespace AM
{

/**
 * Sent by a client to teleport a player to another player, or to a tile.
 *
 * The server checks the requester's role (see AccountRole), applies the
 * teleport, and responds with a TeleportResponse. This only affects this
 * world, so AccountServer isn't involved.
 */
struct TeleportRequest {
    // The MessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::TeleportRequest};

    //--------------------------------------------------------------------------
    // Networked data
    //--------------------------------------------------------------------------
    /** The username of the player to teleport. If empty, the requester
        teleports themselves. */
    std::string targetUsername{};

    /** Where to teleport the target: another player (by username), or the
        bottom center of a tile. */
    std::variant<std::string, TilePosition> destination{};

    //--------------------------------------------------------------------------
    // Local data
    //--------------------------------------------------------------------------
    /** The handle of the client that sent this message.
        Set by the server, based on which connection the message came
        from (we can't trust the client to ID itself). */
    ConnectionHandle clientHandle{};
};

template<typename S>
void serialize(S& serializer, TeleportRequest& teleportRequest)
{
    serializer.text1b(teleportRequest.targetUsername, USERNAME_MAX_LENGTH);
    serializer.ext(teleportRequest.destination,
                   bitsery::ext::StdVariant{
                       [](S& serializer, std::string& username) {
                           serializer.text1b(username, USERNAME_MAX_LENGTH);
                       },
                       [](S& serializer, TilePosition& tilePosition) {
                           serializer.object(tilePosition);
                       }});
}

} // End namespace AM
