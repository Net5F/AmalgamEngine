#pragma once

#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace AM
{
namespace Client
{

/**
 * Holds the chat's slash commands (e.g. /g, /ban), and routes entered text
 * to the matching command's handler.
 *
 * The engine registers its own commands (e.g. see ModerationCommands), and
 * the project can register any others that it needs (e.g. /dance).
 */
class ChatCommandRegistry
{
public:
    /**
     * The result of handling a chat command.
     */
    struct Result {
        enum class Status {
            /** The command was handled. Any further feedback (e.g. a server
                response) is the handler's responsibility. */
            Success,
            /** The command is missing arguments, or an argument is invalid.
                handle() fills in errorMessage with the command's usage
                info. */
            InvalidUsage,
            /** The command failed. errorMessage says why. */
            Failed
        };

        Status status{Status::Success};

        /** If status != Success, a message to show the user. */
        std::string errorMessage{};
    };

    /**
     * Handles a command. Given the text after the command's name, with
     * leading spaces removed.
     */
    using Handler = std::function<Result(std::string_view)>;

    /**
     * Registers a command.
     *
     * @param name The command's name, including the leading slash, e.g. "/g".
     * @param usage The command's usage info, e.g. "/unban <username>
     *              [reason]". Shown if the handler returns InvalidUsage.
     */
    void registerCommand(std::string_view name, std::string_view usage,
                         Handler handler);

    /**
     * If the given text is a slash command, runs its handler and returns the
     * result. If the command isn't registered, returns a Failed result.
     *
     * @return The result, or nullopt if the text isn't a slash command (i.e.
     *         it's a regular chat message).
     */
    std::optional<Result> handle(std::string_view text) const;

private:
    struct Command {
        std::string name{};
        std::string usage{};
        Handler handler{};
    };

    std::vector<Command> commands;
};

} // namespace Client
} // namespace AM
