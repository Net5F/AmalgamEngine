#include "ChatCommandRegistry.h"
#include "ChatCommandParser.h"
#include "World.h"
#include "AccountRole.h"
#include "AccountRoleHelpers.h"
#include "Log.h"
#include <algorithm>
#include <map>
#include <utility>

namespace AM
{
namespace Client
{

ChatCommandRegistry::ChatCommandRegistry(World& inWorld)
: world{inWorld}
, commands{}
{
    registerCommand(
        "/help", "/help [command]", std::nullopt,
        [this](std::string_view arguments) { return help(arguments); });
}

void ChatCommandRegistry::registerCommand(
    std::string_view name, std::string_view usage,
    std::optional<AccountPermission> requiredPermission, Handler handler)
{
    if (!name.starts_with('/')) {
        LOG_FATAL("Chat command names must start with '/': %s",
                  std::string{name}.c_str());
    }
    if (findCommand(name)) {
        LOG_FATAL("Chat command registered twice: %s",
                  std::string{name}.c_str());
    }

    commands.push_back(Command{.name{name},
                               .usage{usage},
                               .requiredPermission{requiredPermission},
                               .handler{std::move(handler)}});
}

std::optional<ChatCommandRegistry::Result>
    ChatCommandRegistry::handle(std::string_view text) const
{
    std::optional<ChatCommandParser::ParsedCommand> parsedCommand{
        ChatCommandParser::parse(text)};
    if (!parsedCommand) {
        return std::nullopt;
    }

    const Command* command{findCommand(parsedCommand->name)};
    if (!command) {
        return Result{Result::Status::Failed,
                      "Unknown command: " + std::string{parsedCommand->name}};
    }

    Result result{command->handler(parsedCommand->arguments)};
    if (result.status == Result::Status::InvalidUsage) {
        result.message = "Usage: " + command->usage;
    }

    return result;
}

ChatCommandRegistry::Result
    ChatCommandRegistry::help(std::string_view arguments) const
{
    const AccountRoleDef& role{getPlayerRole()};

    auto [commandName, remainingText]
        = ChatCommandParser::splitFirstToken(arguments);
    if (!remainingText.empty()) {
        return {Result::Status::InvalidUsage};
    }

    // If a command was given, show its usage.
    if (!commandName.empty()) {
        // Accept the name with or without the slash (e.g. "/help ban").
        std::string fullName{commandName};
        if (!fullName.starts_with('/')) {
            fullName.insert(0, "/");
        }

        const Command* command{findCommand(fullName)};
        if (!command) {
            return {Result::Status::Failed, "Unknown command: " + fullName};
        }
        else if (!canUse(role, *command)) {
            return {Result::Status::Failed,
                    "You don't have permission to use " + fullName + "."};
        }

        return {Result::Status::Success, "Usage: " + command->usage};
    }

    // Group the commands that this role may use by their required
    // permission. Commands that anyone may use come first.
    std::map<std::optional<AccountPermission>, std::vector<std::string_view>>
        groups{};
    for (const Command& command : commands) {
        if (canUse(role, command)) {
            groups[command.requiredPermission].push_back(command.name);
        }
    }

    // List each group on its own line, e.g. "Ban: /ban, /unban".
    std::string message{};
    for (auto& [permission, names] : groups) {
        std::ranges::sort(names);

        if (!message.empty()) {
            message += "\n";
        }
        message += permission ? getPermissionDisplayName(*permission)
                              : std::string_view{"General"};
        message += ": ";
        for (std::size_t i{0}; i < names.size(); ++i) {
            if (i > 0) {
                message += ", ";
            }
            message += names[i];
        }
    }

    return {Result::Status::Success, std::move(message)};
}

const AccountRoleDef& ChatCommandRegistry::getPlayerRole() const
{
    if (world.registry.valid(world.localPlayerEntity)) {
        if (const AccountRole* accountRole{
                world.registry.try_get<AccountRole>(world.localPlayerEntity)}) {
            return AccountRoleHelpers::getRoleOrDefault(accountRole->roleID);
        }
    }

    return AccountRoleHelpers::getDefaultRole();
}

const ChatCommandRegistry::Command*
    ChatCommandRegistry::findCommand(std::string_view name) const
{
    auto commandIt{std::ranges::find(commands, name, &Command::name)};
    if (commandIt == commands.end()) {
        return nullptr;
    }

    return &(*commandIt);
}

bool ChatCommandRegistry::canUse(const AccountRoleDef& role,
                                 const Command& command) const
{
    return !(command.requiredPermission)
           || role.hasPermission(*(command.requiredPermission));
}

} // namespace Client
} // namespace AM
