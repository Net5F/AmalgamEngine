#pragma once

#include "ChatServiceMessageType.h"
#include "Vector3.h"
#include <SDL3/SDL_stdinc.h>
#include <limits>
#include <vector>

namespace AM
{

/**
 * Sent by the world server to tell the chat server where players are, so it
 * can route local chat.
 *
 * If a player isn't yet known to the chat server, receiving their position
 * marks them as being in the world. They remain in the world until they're
 * included in a PlayersLeftWorld.
 *
 * Note: When the world server connects, the chat server assumes that no
 *       players are in the world. The world server is expected to send the
 *       positions of all of its players after (re)connecting.
 */
struct PlayerPositionsUpdate {
    static constexpr ChatServiceMessageType MESSAGE_TYPE{
        ChatServiceMessageType::PlayerPositionsUpdate};

    struct PlayerPosition {
        /** The ID of the player's account. */
        Sint64 accountID{0};

        /** The player entity's position, in world units. */
        Vector3 position{};
    };

    /** The max number of players that a single message can contain.
        Note: This is a per-message limit, not a limit on the number of
              players. Senders must split larger sets across multiple
              messages. */
    static constexpr std::size_t MAX_PLAYERS_PER_MESSAGE{1000};

    /** The max size, in bytes, of a serialized message. Receivers must allow
        payloads of at least this size.
        Note: Bitsery prefixes containers with a size that may take up to 4
              bytes. */
    static constexpr std::size_t MAX_SERIALIZED_SIZE{
        4 + (MAX_PLAYERS_PER_MESSAGE * (sizeof(Sint64) + (3 * sizeof(float))))};
    static_assert(MAX_SERIALIZED_SIZE <= std::numeric_limits<Uint16>::max(),
                  "A full message must fit in a single frame.");

    /** The players whose positions have changed. */
    std::vector<PlayerPosition> playerPositions{};
};

template<typename S>
void serialize(S& serializer,
               PlayerPositionsUpdate::PlayerPosition& playerPosition)
{
    serializer.value8b(playerPosition.accountID);
    serializer.object(playerPosition.position);
}

template<typename S>
void serialize(S& serializer, PlayerPositionsUpdate& playerPositionsUpdate)
{
    serializer.container(playerPositionsUpdate.playerPositions,
                         PlayerPositionsUpdate::MAX_PLAYERS_PER_MESSAGE);
}

} // End namespace AM
