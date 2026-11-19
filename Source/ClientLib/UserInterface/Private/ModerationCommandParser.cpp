#include "ModerationCommandParser.h"
#include "AccountRoleHelpers.h"
#include <algorithm>
#include <array>
#include <cctype>
#include <charconv>
#include <limits>
#include <string>

namespace AM
{
namespace Client
{

ModerationCommandParser::Result
    ModerationCommandParser::parse(std::string_view text)
{
    struct AccountCommandInfo {
        std::string_view name{};
        AccountModerationAction action{};
        std::string_view usage{};
    };
    static constexpr std::array<AccountCommandInfo, 3> ACCOUNT_COMMANDS{
        {{"/ban", AccountModerationAction::Ban,
          "/ban <username> [duration, e.g. 30m, 12h, 7d, 2w] [reason]"},
         {"/unban", AccountModerationAction::Unban,
          "/unban <username> [reason]"},
         {"/role", AccountModerationAction::SetRole,
          "/role <username> <role> [reason]"}}};

    struct WorldCommandInfo {
        std::string_view name{};
        WorldModerationAction action{};
        std::string_view usage{};
    };
    // TODO: Add the teleport destination to /tp's usage.
    static constexpr std::array<WorldCommandInfo, 1> WORLD_COMMANDS{
        {{"/tp", WorldModerationAction::Teleport, "/tp <username>"}}};

    // Find the command.
    auto [commandName, arguments] = splitFirstToken(text);
    auto accountCommandIt{std::ranges::find(ACCOUNT_COMMANDS, commandName,
                                            &AccountCommandInfo::name)};
    if (accountCommandIt != ACCOUNT_COMMANDS.end()) {
        Result result{parseAccountCommand(accountCommandIt->action, arguments)};
        result.usage = accountCommandIt->usage;
        return result;
    }

    auto worldCommandIt{std::ranges::find(WORLD_COMMANDS, commandName,
                                          &WorldCommandInfo::name)};
    if (worldCommandIt != WORLD_COMMANDS.end()) {
        Result result{parseWorldCommand(worldCommandIt->action, arguments)};
        result.usage = worldCommandIt->usage;
        return result;
    }

    return {};
}

ModerationCommandParser::Result
    ModerationCommandParser::parseAccountCommand(AccountModerationAction action,
                                                 std::string_view arguments)
{
    Result result{};
    result.status = Status::InvalidUsage;

    AccountModerationRequest request{};
    request.action = action;

    // Parse the target.
    auto [targetUsername, remainingText] = splitFirstToken(arguments);
    if (!isValidTargetUsername(targetUsername)) {
        return result;
    }
    request.targetUsername = targetUsername;

    // Parse the action-specific arguments.
    if (request.action == AccountModerationAction::SetRole) {
        auto [roleID, textAfterRole] = splitFirstToken(remainingText);
        if (roleID.empty()) {
            return result;
        }

        // Role IDs are lowercase, so accept any case (e.g. "Builder").
        std::string normalizedRoleID{roleID};
        std::ranges::transform(normalizedRoleID, normalizedRoleID.begin(),
                               [](unsigned char c) { return std::tolower(c); });
        if (!AccountRoleHelpers::findRole(normalizedRoleID)) {
            result.status = Status::InvalidRole;
            return result;
        }

        request.roleID = std::move(normalizedRoleID);
        remainingText = textAfterRole;
    }
    else if (request.action == AccountModerationAction::Ban) {
        // If the next token starts with a digit, it's a duration.
        auto [durationToken, textAfterDuration]
            = splitFirstToken(remainingText);
        if (!durationToken.empty()
            && std::isdigit(static_cast<unsigned char>(durationToken[0]))) {
            std::optional<Sint64> durationS{parseDuration(durationToken)};
            if (!durationS) {
                result.status = Status::InvalidDuration;
                return result;
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

    result.status = Status::Success;
    result.request = std::move(request);
    return result;
}

ModerationCommandParser::Result
    ModerationCommandParser::parseWorldCommand(WorldModerationAction action,
                                               std::string_view arguments)
{
    Result result{};
    result.status = Status::InvalidUsage;

    WorldModerationRequest request{};
    request.action = action;

    // Parse the target.
    auto [targetUsername, remainingText] = splitFirstToken(arguments);
    if (!isValidTargetUsername(targetUsername)) {
        return result;
    }
    request.targetUsername = targetUsername;

    // TODO: When teleporting is implemented, parse the destination from
    //       remainingText.

    result.status = Status::Success;
    result.request = std::move(request);
    return result;
}

bool ModerationCommandParser::isValidTargetUsername(std::string_view username)
{
    return !username.empty()
           && (username.size()
               <= static_cast<std::size_t>(USERNAME_MAX_LENGTH));
}

std::pair<std::string_view, std::string_view>
    ModerationCommandParser::splitFirstToken(std::string_view text)
{
    std::size_t tokenStart{text.find_first_not_of(' ')};
    if (tokenStart == std::string_view::npos) {
        return {};
    }
    text.remove_prefix(tokenStart);

    std::size_t tokenEnd{text.find(' ')};
    if (tokenEnd == std::string_view::npos) {
        return {text, {}};
    }

    std::string_view remainder{text.substr(tokenEnd)};
    std::size_t remainderStart{remainder.find_first_not_of(' ')};
    if (remainderStart == std::string_view::npos) {
        remainder = {};
    }
    else {
        remainder.remove_prefix(remainderStart);
    }

    return {text.substr(0, tokenEnd), remainder};
}

std::optional<Sint64>
    ModerationCommandParser::parseDuration(std::string_view token)
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

} // namespace Client
} // namespace AM
