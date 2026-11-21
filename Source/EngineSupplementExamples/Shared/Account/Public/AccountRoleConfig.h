#pragma once

#include "AccountRoleDefs.h"
#include <array>
#include <string_view>

namespace AM
{

/**
 * The roles that accounts may hold.
 *
 * Every account holds exactly 1 role. New accounts are given the default role.
 * An account can only kick, ban, or change the role of accounts with a lower
 * rank, and can only assign roles with a lower rank than its own.
 *
 * To give an account a role from outside the game (e.g. to create the first
 * SuperAdmin), run: AccountServer --set-role <username> <roleID>
 */
struct AccountRoleConfig {
    /** The role that new accounts are given. Must match a role in ROLES. */
    static constexpr std::string_view DEFAULT_ROLE_ID{"player"};

    static constexpr std::array ROLES{
        AccountRoleDef{
            .id{"player"}, .displayName{"Player"}, .rank{0}, .permissions{0}},
        AccountRoleDef{.id{"builder"},
                       .displayName{"Builder"},
                       .rank{1},
                       .permissions{makePermissions(AccountPermission::Build)}},
        AccountRoleDef{.id{"mod"},
                       .displayName{"Mod"},
                       .rank{2},
                       .permissions{makePermissions(AccountPermission::Ban)}},
        AccountRoleDef{.id{"admin"},
                       .displayName{"Admin"},
                       .rank{3},
                       .permissions{makePermissions(
                           AccountPermission::Ban, AccountPermission::Build,
                           AccountPermission::SetRole)}},
        AccountRoleDef{.id{"superadmin"},
                       .displayName{"SuperAdmin"},
                       .rank{4},
                       .permissions{makePermissions(
                           AccountPermission::Ban, AccountPermission::Build,
                           AccountPermission::SetRole)}},
    };
};

} // End namespace AM
