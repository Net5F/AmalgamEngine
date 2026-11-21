#pragma once

#include "AccountRoleDefs.h"
#include <string_view>

namespace AM
{

/**
 * Helper functions for looking up the project's account roles (see
 * AccountRoleConfig.h).
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
};

} // End namespace AM
