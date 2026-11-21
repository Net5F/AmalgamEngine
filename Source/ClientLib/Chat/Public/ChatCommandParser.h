#pragma once

#include <optional>
#include <string_view>
#include <utility>

namespace AM
{
namespace Client
{

/**
 * Splits chat text into slash commands and their arguments.
 *
 * This doesn't know about any specific commands. See ChatCommandRegistry.
 */
class ChatCommandParser
{
public:
    struct ParsedCommand {
        /** The command's name, including the leading slash, e.g. "/ban". */
        std::string_view name{};

        /** Everything after the name, with leading spaces removed. */
        std::string_view arguments{};
    };

    /**
     * If the given text is a slash command, returns its name and arguments.
     * Otherwise, returns nullopt.
     */
    static std::optional<ParsedCommand> parse(std::string_view text);

    /**
     * Splits the given text into its first space-separated token and the
     * rest of the text (with leading spaces removed).
     *
     * Useful for command handlers that need to parse their arguments.
     */
    static std::pair<std::string_view, std::string_view>
        splitFirstToken(std::string_view text);
};

} // namespace Client
} // namespace AM
