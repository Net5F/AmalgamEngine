#pragma once

#include "AccountModerationRequest.h"
#include "WorldModerationRequest.h"
#include <SDL3/SDL_stdinc.h>
#include <optional>
#include <string_view>
#include <utility>
#include <variant>

namespace AM
{
namespace Client
{

/**
 * Parses moderation chat commands into requests.
 *
 * Account-level commands produce an AccountModerationRequest, which should be
 * sent to AccountServer (see AccountSession::sendModerationRequest()):
 *   /ban <username> [duration] [reason]
 *     Duration is a number followed by a unit (m, h, d, or w), e.g. "7d".
 *     If no duration is given, the ban is permanent.
 *   /unban <username> [reason]
 *   /role <username> <roleID> [reason]
 *
 * World-level commands produce a WorldModerationRequest, which should be sent
 * to the world server:
 *   /tp <username>
 *     TODO: Not implemented yet. The server will respond with NotPermitted.
 *
 * The server checks the requester's permissions, so these can be offered to
 * everyone.
 */
class ModerationCommandParser
{
public:
    enum class Status {
        /** The text isn't a moderation command. */
        NotACommand,
        /** The command was parsed successfully. */
        Success,
        /** The command is missing arguments, or an argument is invalid. */
        InvalidUsage,
        /** (Ban only) The duration couldn't be parsed, or is out of range. */
        InvalidDuration,
        /** (Role only) The role doesn't exist. */
        InvalidRole
    };

    struct Result {
        Status status{Status::NotACommand};

        /** If status == Success, the request to send. */
        std::variant<std::monostate, AccountModerationRequest,
                     WorldModerationRequest>
            request{};

        /** If status != NotACommand, the command's usage info, e.g.
            "/unban <username> [reason]". */
        std::string_view usage{};
    };

    /**
     * Parses the given chat text.
     */
    static Result parse(std::string_view text);

private:
    /**
     * Parses the arguments of an account-level command.
     */
    static Result parseAccountCommand(AccountModerationAction action,
                                      std::string_view arguments);

    /**
     * Parses the arguments of a world-level command.
     */
    static Result parseWorldCommand(WorldModerationAction action,
                                    std::string_view arguments);

    /**
     * Returns true if the given username could be a valid target.
     */
    static bool isValidTargetUsername(std::string_view username);

    /**
     * Splits the given text into its first space-separated token and the
     * rest of the text (with leading spaces removed).
     */
    static std::pair<std::string_view, std::string_view>
        splitFirstToken(std::string_view text);

    /**
     * Parses the given duration (e.g. "30m", "12h", "7d", "2w") into seconds.
     *
     * @return The duration in seconds, or nullopt if it's invalid or longer
     *         than MAX_BAN_DURATION_S.
     */
    static std::optional<Sint64> parseDuration(std::string_view token);
};

} // namespace Client
} // namespace AM
