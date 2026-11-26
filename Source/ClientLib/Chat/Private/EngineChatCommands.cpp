#include "EngineChatCommands.h"
#include "ChatCommandParser.h"
#include "AccountSession.h"
#include "Network.h"
#include "AccountRoleHelpers.h"
#include "AccountRoleConfig.h"
#include "BanRequest.h"
#include <algorithm>
#include <cctype>
#include <charconv>
#include <string>
#include <utility>
#include <vector>

namespace AM
{
namespace Client
{

EngineChatCommands::EngineChatCommands(AccountSession& inAccountSession,
                                       Network& inNetwork)
: accountSession{inAccountSession}
, network{inNetwork}
{
}

void EngineChatCommands::registerCommands(ChatCommandRegistry& registry)
{
    registry.registerCommand(
        "/ban", "/ban <username> [duration, e.g. 30m, 12h, 7d, 2w] [reason]",
        AccountPermission::Ban,
        [this](std::string_view arguments) { return ban(arguments); });
    registry.registerCommand(
        "/unban", "/unban <username> [reason]", AccountPermission::Ban,
        [this](std::string_view arguments) { return unban(arguments); });
    registry.registerCommand(
        "/role", "/role <username> <role> [reason]", AccountPermission::SetRole,
        [this](std::string_view arguments) { return setRole(arguments); });

    registry.registerCommand(
        "/tp", "/tp <player> [destination player], or /tp <player> <x> <y> [z]",
        AccountPermission::Teleport,
        [this](std::string_view arguments) { return teleport(arguments); });
}

ChatCommandRegistry::Result EngineChatCommands::ban(std::string_view arguments)
{
    auto [targetUsername, remainingText]
        = ChatCommandParser::splitFirstToken(arguments);
    if (!isValidTargetUsername(targetUsername)) {
        return {ChatCommandRegistry::Result::Status::InvalidUsage};
    }

    // If the next token starts with a digit, it's a duration.
    Sint64 durationS{0};
    auto [durationToken, textAfterDuration]
        = ChatCommandParser::splitFirstToken(remainingText);
    if (!durationToken.empty()
        && std::isdigit(static_cast<unsigned char>(durationToken[0]))) {
        std::optional<Sint64> parsedDurationS{parseDuration(durationToken)};
        if (!parsedDurationS) {
            return {ChatCommandRegistry::Result::Status::Failed,
                    "Invalid ban duration. Use a number followed by m, h, d, "
                    "or w (e.g. 7d), up to 10 years."};
        }

        durationS = *parsedDurationS;
        remainingText = textAfterDuration;
    }

    if (!(accountSession.requestBan(std::string{targetUsername}, durationS,
                                    parseReason(remainingText)))) {
        return getNotLoggedInResult();
    }

    return {};
}

ChatCommandRegistry::Result
    EngineChatCommands::unban(std::string_view arguments)
{
    auto [targetUsername, remainingText]
        = ChatCommandParser::splitFirstToken(arguments);
    if (!isValidTargetUsername(targetUsername)) {
        return {ChatCommandRegistry::Result::Status::InvalidUsage};
    }

    if (!(accountSession.requestUnban(std::string{targetUsername},
                                      parseReason(remainingText)))) {
        return getNotLoggedInResult();
    }

    return {};
}

ChatCommandRegistry::Result
    EngineChatCommands::setRole(std::string_view arguments)
{
    auto [targetUsername, remainingText]
        = ChatCommandParser::splitFirstToken(arguments);
    if (!isValidTargetUsername(targetUsername)) {
        return {ChatCommandRegistry::Result::Status::InvalidUsage};
    }

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

    if (!(accountSession.requestSetRole(std::string{targetUsername},
                                        normalizedRoleID,
                                        parseReason(textAfterRole)))) {
        return getNotLoggedInResult();
    }

    return {};
}

ChatCommandRegistry::Result
    EngineChatCommands::teleport(std::string_view arguments)
{
    TeleportRequest request{};
    if (!parseTeleportArguments(arguments, request)) {
        return {ChatCommandRegistry::Result::Status::InvalidUsage};
    }

    network.worldEndpoint.send(request);
    return {};
}

bool EngineChatCommands::parseTeleportArguments(
    std::string_view arguments, TeleportRequest& teleportRequest)
{
    // Split the arguments into tokens. There may be up to 4.
    constexpr std::size_t MAX_TOKENS{4};
    std::vector<std::string_view> tokens{};
    std::string_view remainingText{arguments};
    while (!remainingText.empty()) {
        auto [token, textAfterToken]
            = ChatCommandParser::splitFirstToken(remainingText);
        if (token.empty()) {
            break;
        }
        else if (tokens.size() == MAX_TOKENS) {
            return false;
        }

        tokens.push_back(token);
        remainingText = textAfterToken;
    }

    if (tokens.size() == 1) {
        // "/tp <player>": Teleport ourselves to the player.
        // Note: An empty target tells the server to teleport us.
        if (!isValidTargetUsername(tokens[0])) {
            return false;
        }
        teleportRequest.destination = std::string{tokens[0]};
    }
    else if (tokens.size() == 2) {
        // "/tp <player> <destination player>"
        if (!isValidTargetUsername(tokens[0])
            || !isValidTargetUsername(tokens[1])) {
            return false;
        }
        teleportRequest.targetUsername = tokens[0];
        teleportRequest.destination = std::string{tokens[1]};
    }
    else if (tokens.size() >= 3) {
        // "/tp <player> <x> <y> [z]"
        std::optional<int> x{parseInt(tokens[1])};
        std::optional<int> y{parseInt(tokens[2])};
        std::optional<int> z{(tokens.size() == 4) ? parseInt(tokens[3])
                                                  : std::optional<int>{0}};
        if (!isValidTargetUsername(tokens[0]) || !x || !y || !z) {
            return false;
        }
        teleportRequest.targetUsername = tokens[0];
        teleportRequest.destination = TilePosition{*x, *y, *z};
    }
    else {
        return false;
    }

    return true;
}

std::string EngineChatCommands::parseReason(std::string_view text)
{
    while (!text.empty() && (text.back() == ' ')) {
        text.remove_suffix(1);
    }

    return std::string{
        text.substr(0, std::min(text.size(), MODERATION_REASON_MAX_LENGTH))};
}

bool EngineChatCommands::isValidTargetUsername(std::string_view username)
{
    return !username.empty()
           && (username.size()
               <= static_cast<std::size_t>(USERNAME_MAX_LENGTH));
}

std::optional<Sint64> EngineChatCommands::parseDuration(std::string_view token)
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
    if (amount > (BanRequest::MAX_DURATION_S / unitS)) {
        return std::nullopt;
    }

    return amount * unitS;
}

std::optional<int> EngineChatCommands::parseInt(std::string_view token)
{
    int value{0};
    const char* tokenEnd{token.data() + token.size()};
    auto [parseEnd, error] = std::from_chars(token.data(), tokenEnd, value);
    if ((error != std::errc{}) || (parseEnd != tokenEnd)) {
        return std::nullopt;
    }

    return value;
}

ChatCommandRegistry::Result EngineChatCommands::getInvalidRoleResult()
{
    std::string message{"Unknown role. Valid roles:"};
    for (const AccountRoleDef& role : AccountRoleConfig::ROLES) {
        message += " ";
        message += role.id;
    }

    return {ChatCommandRegistry::Result::Status::Failed, std::move(message)};
}

ChatCommandRegistry::Result EngineChatCommands::getNotLoggedInResult()
{
    return {ChatCommandRegistry::Result::Status::Failed,
            "You must be logged in to do that."};
}

} // namespace Client
} // namespace AM
