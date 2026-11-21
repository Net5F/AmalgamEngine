#pragma once

#include <SDL3/SDL_stdinc.h>
#include <cstddef>
#include <string_view>

namespace AM
{

/**
 * The actions that an account's role may permit.
 *
 * Each value is a bit, so a role's permissions can be stored as a bitset (see
 * makePermissions()).
 */
enum class AccountPermission : Uint8 {
    /** Ban or unban a lower-ranked account. */
    Ban = 1 << 0,
    /** Modify the world (tiles, entities, items).
        Note: The engine doesn't enforce this. The project's simulation hooks
              decide how it combines with their own rules (see
              WorldServer::ISimulationExtension). */
    Build = 1 << 1,
    /** Change a lower-ranked account's role to a lower-ranked role. */
    SetRole = 1 << 2,
};

/**
 * Combines the given permissions into a bitset, for use in
 * AccountRoleDef::permissions.
 */
template<typename... Permissions>
constexpr Uint8 makePermissions(Permissions... permissions)
{
    return static_cast<Uint8>((static_cast<Uint8>(permissions) | ... | 0));
}

/** The max length of a role's ID. */
inline constexpr std::size_t ACCOUNT_ROLE_ID_MAX_LENGTH{32};

/**
 * A role that an account may hold.
 *
 * Roles are defined by the project, in AccountRoleConfig.h.
 */
struct AccountRoleDef {
    /** The role's unique ID. This is what gets saved to the database, so
        changing it will cause accounts that hold the role to fall back to
        the default role.
        Must be lowercase, and at most ACCOUNT_ROLE_ID_MAX_LENGTH chars. */
    std::string_view id{};

    /** The role's user-facing name. */
    std::string_view displayName{};

    /** Used to decide who can moderate whom. An account can only act on
        accounts whose role has a lower rank than its own, and can only
        assign roles with a lower rank than its own. */
    Uint8 rank{0};

    /** The permissions that this role grants (see makePermissions()). */
    Uint8 permissions{0};

    constexpr bool hasPermission(AccountPermission permission) const
    {
        return (permissions & static_cast<Uint8>(permission)) != 0;
    }

    /**
     * Returns true if this role's rank is higher than the given role's.
     *
     * Note: This doesn't check permissions, only rank.
     */
    constexpr bool outranks(const AccountRoleDef& other) const
    {
        return rank > other.rank;
    }
};

} // End namespace AM
