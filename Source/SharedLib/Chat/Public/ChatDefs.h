#pragma once

#include <SDL3/SDL_stdinc.h>
#include <cstddef>

/**
 * This file contains shared chat type definitions that should be consistent
 * between the chat server and its peers (client, world server).
 *
 * Note: The local chat range is defined in SharedConfig.
 */
namespace AM
{
//--------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------
/** The max length, in bytes, of a chat message's text (arbitrarily chosen). */
inline constexpr std::size_t CHAT_MESSAGE_MAX_LENGTH{200};

/** The channels that a chat message can be sent to. */
enum class ChatChannel : Uint8 {
    /** Sent to every connected player. */
    Global,
    /** Sent to every player within SharedConfig::LOCAL_CHAT_RADIUS of the
        sender. */
    Local,
    // Note: Keep this last.
    Count
};

/**
 * Returns true if the given channel is a real channel value.
 *
 * Channels are received over the network, so they must be checked before use.
 */
inline constexpr bool isValidChatChannel(ChatChannel channel)
{
    return static_cast<Uint8>(channel) < static_cast<Uint8>(ChatChannel::Count);
}

} // End namespace AM
