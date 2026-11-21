#include "AccountRoleHelpers.h"
#include "AccountRoleConfig.h"
#include "Log.h"
#include <algorithm>
#include <string>

namespace AM
{

// Validate the project's role definitions.
static_assert(
    []() {
        for (const AccountRoleDef& role : AccountRoleConfig::ROLES) {
            if (role.id == AccountRoleConfig::DEFAULT_ROLE_ID) {
                return true;
            }
        }
        return false;
    }(),
    "AccountRoleConfig::DEFAULT_ROLE_ID must match a role in ROLES.");
static_assert(
    []() {
        for (std::size_t i{0}; i < AccountRoleConfig::ROLES.size(); ++i) {
            const AccountRoleDef& role{AccountRoleConfig::ROLES[i]};
            if (role.id.empty()
                || (role.id.size() > ACCOUNT_ROLE_ID_MAX_LENGTH)) {
                return false;
            }
            for (char c : role.id) {
                if ((c >= 'A') && (c <= 'Z')) {
                    return false;
                }
            }
            for (std::size_t j{i + 1}; j < AccountRoleConfig::ROLES.size();
                 ++j) {
                if (role.id == AccountRoleConfig::ROLES[j].id) {
                    return false;
                }
            }
        }
        return true;
    }(),
    "AccountRoleConfig::ROLES IDs must be unique, non-empty, lowercase, and at "
    "most ACCOUNT_ROLE_ID_MAX_LENGTH chars.");

const AccountRoleDef* AccountRoleHelpers::findRole(std::string_view roleID)
{
    auto roleIt{std::ranges::find(AccountRoleConfig::ROLES, roleID,
                                  &AccountRoleDef::id)};
    if (roleIt == AccountRoleConfig::ROLES.end()) {
        return nullptr;
    }

    return &(*roleIt);
}

const AccountRoleDef& AccountRoleHelpers::getDefaultRole()
{
    // Note: Our static_assert guarantees that the default role exists.
    return *findRole(AccountRoleConfig::DEFAULT_ROLE_ID);
}

const AccountRoleDef&
    AccountRoleHelpers::getRoleOrDefault(std::string_view roleID)
{
    if (const AccountRoleDef* role{findRole(roleID)}) {
        return *role;
    }

    LOG_WARN("Unknown account role: \"%s\". Using the default role.",
             std::string{roleID}.c_str());
    return getDefaultRole();
}

bool AccountRoleHelpers::canActOn(const AccountRoleDef& actorRole,
                                  const AccountRoleDef& targetRole)
{
    return actorRole.rank > targetRole.rank;
}

bool AccountRoleHelpers::canAssign(const AccountRoleDef& actorRole,
                                   const AccountRoleDef& newRole)
{
    return actorRole.rank > newRole.rank;
}

} // End namespace AM
