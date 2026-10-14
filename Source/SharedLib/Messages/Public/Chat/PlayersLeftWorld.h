#pragma once

#include "ChatServiceMessageType.h"
#include <SDL3/SDL_stdinc.h>
#include <limits>
#include <vector>

namespace AM
{

/**
 * Sent by the world server to tell the chat server that players have left
 * the world (e.g. their entity was removed after logging out).
 *
 * These players will no longer be able to send or receive local chat, until
 * the world server sends their position again.
 */
struct PlayersLeftWorld {
    static constexpr ChatServiceMessageType MESSAGE_TYPE{
        ChatServiceMessageType::PlayersLeftWorld};

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
        4 + (MAX_PLAYERS_PER_MESSAGE * sizeof(Sint64))};
    static_assert(MAX_SERIALIZED_SIZE <= std::numeric_limits<Uint16>::max(),
                  "A full message must fit in a single frame.");

    /** The account IDs of the players that left. */
    std::vector<Sint64> accountIDs{};
};

template<typename S>
void serialize(S& serializer, PlayersLeftWorld& playersLeftWorld)
{
    serializer.container8b(playersLeftWorld.accountIDs,
                           PlayersLeftWorld::MAX_PLAYERS_PER_MESSAGE);
}

} // End namespace AM
