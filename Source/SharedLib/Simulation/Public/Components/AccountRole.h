#pragma once

#include "AccountRoleDefs.h"
#include "AccountRoleConfig.h"
#include <string>

namespace AM
{

/**
 * The role held by a player entity's account (see AccountRoleConfig.h).
 *
 * Set by the world server when the account's client connects, and updated if
 * the account's role changes while it's online. Replicated to the owning
 * client, so it can show or hide the features that its role permits.
 *
 * Note: This isn't persisted. AccountServer is the source of truth for
 *       roles.
 */
struct AccountRole {
    /** The ID of the role. Use AccountRoleHelpers::getRoleOrDefault() to get
        its definition. */
    std::string roleID{AccountRoleConfig::DEFAULT_ROLE_ID};
};

template<typename S>
void serialize(S& serializer, AccountRole& accountRole)
{
    serializer.text1b(accountRole.roleID, ACCOUNT_ROLE_ID_MAX_LENGTH);
}

} // namespace AM
