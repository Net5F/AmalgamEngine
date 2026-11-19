#include "CommandLineTool.h"
#include "Database.h"
#include "AccountRoleHelpers.h"
#include "AccountRoleDefs.h"
#include <algorithm>
#include <cctype>
#include <cstdio>

namespace AM
{
namespace AccountServer
{

CommandLineTool::CommandLineTool(std::span<char*> inArgs)
: args{inArgs}
{
}

int CommandLineTool::run()
{
    std::string_view command{args.empty() ? "" : args[0]};
    if ((command == "--set-role") && (args.size() == 3)) {
        return setRole(args[1], args[2]);
    }

    printUsage();
    return 1;
}

int CommandLineTool::setRole(const std::string& username,
                             std::string_view roleID)
{
    // Role IDs are lowercase, so accept any case (e.g. "SuperAdmin").
    std::string normalizedRoleID{roleID};
    std::ranges::transform(normalizedRoleID, normalizedRoleID.begin(),
                           [](unsigned char c) { return std::tolower(c); });
    const AccountRoleDef* role{AccountRoleHelpers::findRole(normalizedRoleID)};
    if (!role) {
        std::printf("Unknown role: \"%s\".\n", std::string{roleID}.c_str());
        printRoles();
        return 1;
    }

    Database database{};
    Database::ConsoleSetRoleResult result{
        database.setRoleFromConsole(username, std::string{role->id})};
    switch (result.result) {
        case Database::ConsoleSetRoleResult::Result::Success: {
            std::printf("Changed %s's role: %s -> %s.\n",
                        result.username.c_str(), result.previousRoleID.c_str(),
                        std::string{role->id}.c_str());
            std::printf("If they're in the world, the change will take effect "
                        "the next time they connect.\n");
            return 0;
        }
        case Database::ConsoleSetRoleResult::Result::AccountNotFound: {
            std::printf("No account has the username \"%s\".\n",
                        username.c_str());
            return 1;
        }
        default: {
            std::printf("Failed to change role: database error. See "
                        "AccountServer.log for details.\n");
            return 1;
        }
    }
}

void CommandLineTool::printUsage()
{
    std::printf("Usage:\n");
    std::printf("  AccountServer\n");
    std::printf("    Runs the server.\n");
    std::printf("  AccountServer --set-role <username> <roleID>\n");
    std::printf("    Gives the account the given role.\n");
    printRoles();
}

void CommandLineTool::printRoles()
{
    std::printf("Valid roles:");
    for (const AccountRoleDef& role : AccountRoleDefs::ROLES) {
        std::printf(" %s", std::string{role.id}.c_str());
    }
    std::printf("\n");
}

} // End namespace AccountServer
} // End namespace AM
