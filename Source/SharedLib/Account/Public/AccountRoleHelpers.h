#pragma once

#include "AccountRoleDef.h"
#include <string_view>

namespace AM
{

/**
 * Helper functions for looking up the project's account roles (see
 * AccountRoleConfig.h) and applying our moderation rules.
 */
class AccountRoleHelpers
{
public:
    /**
     * Returns the role with the given ID, or nullptr if there isn't one.
     */
    static const AccountRoleDef* findRole(std::string_view roleID);

    /**
     * Returns the role that new accounts are given.
     */
    static const AccountRoleDef& getDefaultRole();

    /**
     * Returns the role with the given ID. If there isn't one (e.g. the
     * project removed it), returns the default role.
     */
    static const AccountRoleDef& getRoleOrDefault(std::string_view roleID);

    /**
     * Returns true if the given actor may kick, ban, unban, or change the
     * role of an account that holds the given target role.
     *
     * Note: This doesn't check the actor's permissions, only rank.
     */
    static bool canActOn(const AccountRoleDef& actorRole,
                         const AccountRoleDef& targetRole);

    /**
     * Returns true if the given actor may assign the given role to an
     * account.
     *
     * Note: This doesn't check the actor's permissions, only rank.
     */
    static bool canAssign(const AccountRoleDef& actorRole,
                          const AccountRoleDef& newRole);
};

} // End namespace AM
