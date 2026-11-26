#pragma once

#include "EngineMessageType.h"
#include "AccountDefs.h"
#include "TilePosition.h"
#include "bitsery/ext/std_variant.h"
#include <SDL3/SDL_stdinc.h>
#include <string>
#include <variant>

namespace AM
{

/**
 * Sent by the server in response to a TeleportRequest.
 */
struct TeleportResponse {
    // The MessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::TeleportResponse};

    enum Result : Uint8 {
        Success,
        /** The requester lacks the Teleport permission, or doesn't outrank
            the target. */
        NotPermitted,
        /** The target isn't in the world. */
        TargetNotOnline,
        /** The destination player isn't in the world. */
        DestinationNotOnline,
        /** The destination tile is outside of the tile map. */
        InvalidDestination,
        /** The teleport failed for some other reason. */
        InternalError
    };
    Result result{Result::InternalError};

    /** The target's username, as given in the request. If empty, the
        requester teleported themselves. */
    std::string targetUsername{};

    /** The destination, as given in the request. */
    std::variant<std::string, TilePosition> destination{};
};

template<typename S>
void serialize(S& serializer, TeleportResponse& teleportResponse)
{
    serializer.value1b(teleportResponse.result);
    serializer.text1b(teleportResponse.targetUsername, USERNAME_MAX_LENGTH);
    serializer.ext(teleportResponse.destination,
                   bitsery::ext::StdVariant{
                       [](S& serializer, std::string& username) {
                           serializer.text1b(username, USERNAME_MAX_LENGTH);
                       },
                       [](S& serializer, TilePosition& tilePosition) {
                           serializer.object(tilePosition);
                       }});
}

} // End namespace AM
