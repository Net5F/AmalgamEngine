#include "ModerationCommands.h"
#include "ChatCommandParser.h"
#include "AccountSession.h"
#include "Network.h"
#include "AccountRoleHelpers.h"
#include "AccountRoleConfig.h"
#include <algorithm>
#include <cctype>
#include <charconv>
#include <string>
#include <utility>

namespace AM
{
namespace Client
{

ModerationCommands::ModerationCommands(AccountSession& inAccountSession,
                                       Network& inNetwork)
: accountSession{inAccountSession}
, network{inNetwork}
{
}

void ModerationCommands::registerCommands(ChatCommandRegistry& registry)
{
    registry.registerCommand(
        "/ban", "/ban <username> [duration, e.g. 30m, 12h, 7d, 2w] [reason]",
        [this](std::string_view arguments) {
            return sendAccountRequest(AccountModerationAction::Ban, arguments);
        });
    registry.registerCommand("/unban", "/unban <username> [reason]",
                             [this](std::string_view arguments) {
                                 return sendAccountRequest(
                                     AccountModerationAction::Unban, arguments);
                             });
    registry.registerCommand("/role", "/role <username> <role> [reason]",
                             [this](std::string_view arguments) {
                                 return sendAccountRequest(
                                     AccountModerationAction::SetRole,
                                     arguments);
                             });

    // TODO: Add the teleport destination to /tp's usage.
    registry.registerCommand(
        "/tp", "/tp <username>", [this](std::string_view arguments) {
            return sendWorldRequest(WorldModerationAction::Teleport, arguments);
        });
}

ChatCommandRegistry::Result
    ModerationCommands::sendAccountRequest(AccountModerationAction action,
                                           std::string_view arguments)
{
    AccountModerationRequest request{};
    request.action = action;

    // Parse the target.
    auto [targetUsername, remainingText]
        = ChatCommandParser::splitFirstToken(arguments);
    if (!isValidTargetUsername(targetUsername)) {
        return {ChatCommandRegistry::Result::Status::InvalidUsage};
    }
    request.targetUsername = targetUsername;

    // Parse the action-specific arguments.
    if (request.action == AccountModerationAction::SetRole) {
        auto [roleID, textAfterRole]
            = ChatCommandParser::splitFirstToken(remainingText);
        if (roleID.empty()) {
            return {ChatCommandRegistry::Result::Status::InvalidUsage};
        }

        // Role IDs are lowercase, so accept any case (e.g. "Builder").
        std::string normalizedRoleID{roleID};
        std::ranges::transform(normalizedRoleID, normalizedRoleID.begin(),
                               [](unsigned char c) { return std::tolower(c); });
        if (!AccountRoleHelpers::findRole(normalizedRoleID)) {
            return getInvalidRoleResult();
        }

        request.roleID = std::move(normalizedRoleID);
        remainingText = textAfterRole;
    }
    else if (request.action == AccountModerationAction::Ban) {
        // If the next token starts with a digit, it's a duration.
        auto [durationToken, textAfterDuration]
            = ChatCommandParser::splitFirstToken(remainingText);
        if (!durationToken.empty()
            && std::isdigit(static_cast<unsigned char>(durationToken[0]))) {
            std::optional<Sint64> durationS{parseDuration(durationToken)};
            if (!durationS) {
                return {ChatCommandRegistry::Result::Status::Failed,
                        "Invalid ban duration. Use a number followed by m, h, "
                        "d, or w (e.g. 7d), up to 10 years."};
            }

            request.durationS = *durationS;
            remainingText = textAfterDuration;
        }
    }

    // Whatever's left is the reason.
    while (!remainingText.empty() && (remainingText.back() == ' ')) {
        remainingText.remove_suffix(1);
    }
    request.reason = remainingText.substr(
        0, std::min(remainingText.size(), MODERATION_REASON_MAX_LENGTH));

    if (!(accountSession.sendModerationRequest(std::move(request)))) {
        return {ChatCommandRegistry::Result::Status::Failed,
                "You must be logged in to do that."};
    }

    return {};
}

ChatCommandRegistry::Result
    ModerationCommands::sendWorldRequest(WorldModerationAction action,
                                         std::string_view arguments)
{
    WorldModerationRequest request{};
    request.action = action;

    // Parse the target.
    auto [targetUsername, remainingText]
        = ChatCommandParser::splitFirstToken(arguments);
    if (!isValidTargetUsername(targetUsername)) {
        return {ChatCommandRegistry::Result::Status::InvalidUsage};
    }
    request.targetUsername = targetUsername;

    // TODO: When teleporting is implemented, parse the destination from
    //       remainingText.

    network.worldEndpoint.send(request);
    return {};
}

bool ModerationCommands::isValidTargetUsername(std::string_view username)
{
    return !username.empty()
           && (username.size()
               <= static_cast<std::size_t>(USERNAME_MAX_LENGTH));
}

std::optional<Sint64> ModerationCommands::parseDuration(std::string_view token)
{
    // Parse the number.
    Sint64 amount{0};
    const char* tokenEnd{token.data() + token.size()};
    auto [unitStart, error] = std::from_chars(token.data(), tokenEnd, amount);
    if ((error != std::errc{}) || (amount <= 0)) {
        return std::nullopt;
    }

    // Parse the unit (exactly 1 char).
    if ((tokenEnd - unitStart) != 1) {
        return std::nullopt;
    }
    Sint64 unitS{0};
    switch (std::tolower(static_cast<unsigned char>(*unitStart))) {
        case 'm': {
            unitS = 60;
            break;
        }
        case 'h': {
            unitS = 60 * 60;
            break;
        }
        case 'd': {
            unitS = 60 * 60 * 24;
            break;
        }
        case 'w': {
            unitS = 60 * 60 * 24 * 7;
            break;
        }
        default: {
            return std::nullopt;
        }
    }

    // Note: We check before multiplying, to avoid overflow.
    if (amount > (MAX_BAN_DURATION_S / unitS)) {
        return std::nullopt;
    }

    return amount * unitS;
}

ChatCommandRegistry::Result ModerationCommands::getInvalidRoleResult()
{
    std::string message{"Unknown role. Valid roles:"};
    for (const AccountRoleDef& role : AccountRoleConfig::ROLES) {
        message += " ";
        message += role.id;
    }

    return {ChatCommandRegistry::Result::Status::Failed, std::move(message)};
}

} // namespace Client
} // namespace AM
