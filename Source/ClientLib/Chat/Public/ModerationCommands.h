#pragma once

#include "ChatCommandRegistry.h"
#include "AccountModerationRequest.h"
#include "WorldModerationRequest.h"
#include <SDL3/SDL_stdinc.h>
#include <optional>
#include <string_view>

namespace AM
{
namespace Client
{
class AccountSession;
class Network;

/**
 * The moderation-related chat commands.
 *
 * Account-level commands send an AccountModerationRequest to AccountServer
 * (see AccountSession::sendModerationRequest()):
 *   /ban <username> [duration] [reason]
 *     Duration is a number followed by a unit (m, h, d, or w), e.g. "7d".
 *     If no duration is given, the ban is permanent.
 *   /unban <username> [reason]
 *   /role <username> <roleID> [reason]
 *
 * World-level commands send a WorldModerationRequest to the world server:
 *   /tp <username>
 *     TODO: Not registered yet, since the server doesn't implement it. See
 *           registerCommands().
 *
 * The server checks the requester's permissions, so these can be offered to
 * everyone. The server's responses are delivered through
 * AccountSession::moderationRequestCompleted and the network event
 * dispatcher (WorldModerationResponse).
 *
 * Note: The registered handlers reference this object, so it must outlive
 *       the registry that they're registered to.
 */
class ModerationCommands
{
public:
    ModerationCommands(AccountSession& inAccountSession, Network& inNetwork);

    /**
     * Registers our commands to the given registry.
     */
    void registerCommands(ChatCommandRegistry& registry);

private:
    /**
     * Parses the arguments of an account-level command and sends the
     * request.
     */
    ChatCommandRegistry::Result
        sendAccountRequest(AccountModerationAction action,
                           std::string_view arguments);

    /**
     * Parses the arguments of a world-level command and sends the request.
     */
    ChatCommandRegistry::Result sendWorldRequest(WorldModerationAction action,
                                                 std::string_view arguments);

    /**
     * Returns true if the given username could be a valid target.
     */
    bool isValidTargetUsername(std::string_view username);

    /**
     * Parses the given duration (e.g. "30m", "12h", "7d", "2w") into seconds.
     *
     * @return The duration in seconds, or nullopt if it's invalid or longer
     *         than MAX_BAN_DURATION_S.
     */
    std::optional<Sint64> parseDuration(std::string_view token);

    /**
     * Returns a failure message that lists the valid roles.
     */
    ChatCommandRegistry::Result getInvalidRoleResult();

    /** Used to send account-level moderation requests. */
    AccountSession& accountSession;

    /** Used to send world-level moderation requests. */
    Network& network;
};

} // namespace Client
} // namespace AM
