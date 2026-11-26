#pragma once

#include "ChatCommandRegistry.h"
#include "TeleportRequest.h"
#include <SDL3/SDL_stdinc.h>
#include <optional>
#include <string>
#include <string_view>

namespace AM
{
namespace Client
{
class AccountSession;
class Network;

/**
 * The engine's built-in chat commands (other than /help, which
 * ChatCommandRegistry registers itself). The project registers its own
 * commands separately.
 *
 * Account commands, sent to AccountServer through AccountSession:
 *   /ban <username> [duration] [reason]
 *     Duration is a number followed by a unit (m, h, d, or w), e.g. "7d". If
 *     no duration is given, the ban is permanent.
 *   /unban <username> [reason]
 *   /role <username> <roleID> [reason]
 *
 * World commands, sent to the world server:
 *   /tp <player>
 *     Teleports you to the given player.
 *   /tp <player> <destination player>
 *     Teleports the first player to the second.
 *   /tp <player> <x> <y> [z]
 *     Teleports the player to the given tile. If no z is given, uses 0.
 *
 * The servers check the requester's permissions, so these can be offered to
 * everyone. The servers' responses are delivered through AccountSession's
 * xyzRequestCompleted signals, and the network event dispatcher (e.g.
 * TeleportResponse).
 *
 * Note: The registered handlers reference this object, so it must outlive
 *       the registry that they're registered to.
 */
class EngineChatCommands
{
public:
    EngineChatCommands(AccountSession& inAccountSession, Network& inNetwork);

    /**
     * Registers our commands to the given registry.
     */
    void registerCommands(ChatCommandRegistry& registry);

private:
    /**
     * Handles /ban <username> [duration] [reason].
     */
    ChatCommandRegistry::Result ban(std::string_view arguments);

    /**
     * Handles /unban <username> [reason].
     */
    ChatCommandRegistry::Result unban(std::string_view arguments);

    /**
     * Handles /role <username> <roleID> [reason].
     */
    ChatCommandRegistry::Result setRole(std::string_view arguments);

    /**
     * Handles /tp <player> [destination player], or
     * /tp <player> <x> <y> [z].
     */
    ChatCommandRegistry::Result teleport(std::string_view arguments);

    /**
     * Parses the arguments of a /tp command into the given request.
     *
     * @return true if the arguments are valid, else false.
     */
    bool parseTeleportArguments(std::string_view arguments,
                                TeleportRequest& teleportRequest);

    /**
     * Returns the given text as a moderation action's reason: trailing spaces
     * are removed, and it's truncated to MODERATION_REASON_MAX_LENGTH.
     */
    std::string parseReason(std::string_view text);

    /**
     * Returns true if the given username could be a valid target.
     */
    bool isValidTargetUsername(std::string_view username);

    /**
     * Parses the given duration (e.g. "30m", "12h", "7d", "2w") into seconds.
     *
     * @return The duration in seconds, or nullopt if it's invalid or longer
     *         than BanRequest::MAX_DURATION_S.
     */
    std::optional<Sint64> parseDuration(std::string_view token);

    /**
     * Parses the given token as a whole integer, e.g. "-12".
     *
     * @return The integer, or nullopt if the token isn't one.
     */
    std::optional<int> parseInt(std::string_view token);

    /**
     * Returns a failure message that lists the valid roles.
     */
    ChatCommandRegistry::Result getInvalidRoleResult();

    /**
     * Returns a failure message for an account command that couldn't be sent
     * because we aren't logged in.
     */
    ChatCommandRegistry::Result getNotLoggedInResult();

    /** Used to send account requests (e.g. BanRequest). */
    AccountSession& accountSession;

    /** Used to send world requests (e.g. TeleportRequest). */
    Network& network;
};

} // namespace Client
} // namespace AM
