#include "ChatCommandRegistry.h"
#include "ChatCommandParser.h"
#include "Log.h"
#include <algorithm>
#include <utility>

namespace AM
{
namespace Client
{

void ChatCommandRegistry::registerCommand(std::string_view name,
                                          std::string_view usage,
                                          Handler handler)
{
    if (!name.starts_with('/')) {
        LOG_FATAL("Chat command names must start with '/': %s",
                  std::string{name}.c_str());
    }
    if (std::ranges::find(commands, name, &Command::name) != commands.end()) {
        LOG_FATAL("Chat command registered twice: %s",
                  std::string{name}.c_str());
    }

    commands.push_back(
        Command{.name{name}, .usage{usage}, .handler{std::move(handler)}});
}

std::optional<ChatCommandRegistry::Result>
    ChatCommandRegistry::handle(std::string_view text) const
{
    std::optional<ChatCommandParser::ParsedCommand> parsedCommand{
        ChatCommandParser::parse(text)};
    if (!parsedCommand) {
        return std::nullopt;
    }

    auto commandIt{
        std::ranges::find(commands, parsedCommand->name, &Command::name)};
    if (commandIt == commands.end()) {
        return Result{Result::Status::Failed,
                      "Unknown command: " + std::string{parsedCommand->name}};
    }

    Result result{commandIt->handler(parsedCommand->arguments)};
    if (result.status == Result::Status::InvalidUsage) {
        result.errorMessage = "Usage: " + commandIt->usage;
    }

    return result;
}

} // namespace Client
} // namespace AM
