#pragma once

#include <span>
#include <string>
#include <string_view>

namespace AM
{
namespace AccountServer
{

/**
 * Runs one-shot administrative commands against the account database, instead
 * of starting the server.
 *
 * Usage:
 *   AccountServer --set-role <username> <roleID>
 *     Gives the account the given role, without any permission checks. Used
 *     to create the first SuperAdmin, or to recover if no account can assign
 *     roles anymore.
 *
 * These commands are safe to run while the server is running. Their changes
 * take effect the next time the affected account connects to the world.
 */
class CommandLineTool
{
public:
    /**
     * @param inArgs The command line arguments, excluding the executable
     *               name.
     */
    explicit CommandLineTool(std::span<char*> inArgs);

    /**
     * Runs the given command.
     *
     * @return The process exit code (0 on success).
     */
    int run();

private:
    /**
     * Runs the --set-role command.
     */
    int setRole(const std::string& username, std::string_view roleID);

    /**
     * Prints our usage info.
     */
    void printUsage();

    /**
     * Prints the IDs of all valid roles.
     */
    void printRoles();

    std::span<char*> args;
};

} // End namespace AccountServer
} // End namespace AM
