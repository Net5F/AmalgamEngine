#pragma once

#include "AccountRoleDefs.h"
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace AM
{
namespace Client
{
class World;

/**
 * Holds the chat's slash commands (e.g. /g, /ban), and routes entered text
 * to the matching command's handler.
 *
 * The engine registers its own commands (e.g. /help, and see
 * ModerationCommands), and the project can register any others that it needs
 * (e.g. /dance).
 */
class ChatCommandRegistry
{
public:
    /**
     * The result of handling a chat command.
     */
    struct Result {
        enum class Status {
            /** The command was handled. If message isn't empty, it should be
                shown to the user. Any further feedback (e.g. a server
                response) is the handler's responsibility. */
            Success,
            /** The command is missing arguments, or an argument is invalid.
                handle() fills in message with the command's usage info. */
            InvalidUsage,
            /** The command failed. message says why. */
            Failed
        };

        Status status{Status::Success};

        /** A message to show the user. Required if status == Failed, optional
            otherwise. May contain newlines. */
        std::string message{};
    };

    /**
     * Handles a command. Given the text after the command's name, with
     * leading spaces removed.
     */
    using Handler = std::function<Result(std::string_view arguments)>;

    /**
     * Registers our built-in commands (/help).
     */
    ChatCommandRegistry(World& inWorld);

    // Note: Our /help handler's lambda captures 'this', so a copy's /help
    //       would still refer to the copied-from registry.
    ChatCommandRegistry(const ChatCommandRegistry&) = delete;
    ChatCommandRegistry& operator=(const ChatCommandRegistry&) = delete;

    /**
     * Registers a command.
     *
     * @param name The command's name, including the leading slash, e.g. "/g".
     * @param usage The command's usage info, e.g. "/unban <username>
     *              [reason]". Shown if the handler returns InvalidUsage, and
     *              by "/help <command>".
     * @param requiredPermission The permission that the user's role must
     *                           have for /help to list this command, or
     *                           nullopt if everyone may use it.
     *                           Note: This only affects /help. The server
     *                           is responsible for enforcing permissions.
     */
    void registerCommand(std::string_view name, std::string_view usage,
                         std::optional<AccountPermission> requiredPermission,
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
        std::optional<AccountPermission> requiredPermission{};
        Handler handler{};
    };

    /**
     * Handles /help [command].
     *
     * With no arguments, lists the commands that the player's role may use,
     * grouped by their required permission. Given a command, shows its usage.
     */
    Result help(std::string_view arguments) const;

    /**
     * Returns the player's account role. If the player entity doesn't exist
     * yet, returns the default role.
     */
    const AccountRoleDef& getPlayerRole() const;

    /**
     * Returns the given command, or nullptr if it isn't registered.
     */
    const Command* findCommand(std::string_view name) const;

    /**
     * Returns true if the given role may use the given command.
     */
    bool canUse(const AccountRoleDef& role, const Command& command) const;

    /** Used to get the player's account role. */
    World& world;

    std::vector<Command> commands;
};

} // namespace Client
} // namespace AM
