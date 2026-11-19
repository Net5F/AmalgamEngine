#pragma once

#include <SDL3/SDL_stdinc.h>
#include <cstddef>

/**
 * This file contains moderation type definitions that are shared between the
 * client, world server, and account server.
 *
 * Moderation actions are split by where they're handled:
 *   AccountModerationAction: Account-level actions (bans, roles). The client
 *     sends these directly to AccountServer, which applies them and notifies
 *     the other services.
 *   WorldModerationAction: World-level actions (e.g. teleporting a player).
 *     The client sends these to the world server, which checks the
 *     requester's role and applies them itself.
 */
namespace AM
{

/** The max length of a moderation action's reason. */
inline constexpr std::size_t MODERATION_REASON_MAX_LENGTH{200};

/** The longest ban that can be given, in seconds (10 years). For anything
    longer, use a permanent ban. */
inline constexpr Sint64 MAX_BAN_DURATION_S{60LL * 60 * 24 * 365 * 10};

/** An account-level moderation action, handled by AccountServer. */
enum class AccountModerationAction : Uint8 {
    /** Ban the target, either permanently or for a duration. Disconnects
        their clients and logs them out everywhere. */
    Ban,
    /** Lift the target's active ban. */
    Unban,
    /** Change the target's role. */
    SetRole,
    Count
};

/** A world-level moderation action, handled by the world server. */
enum class WorldModerationAction : Uint8 {
    /** Teleport the target. */
    Teleport,
    Count
};

/** The result of a moderation request. */
enum class ModerationResult : Uint8 {
    Success,
    /** The requester lacks the needed permission, or doesn't outrank the
        target (or the role being assigned). */
    NotPermitted,
    /** (Account actions only) The requester's session is invalid or
        expired. */
    InvalidSession,
    /** No account has the given username. */
    TargetNotFound,
    /** (World actions only) The target isn't in the world. */
    TargetNotOnline,
    /** (SetRole only) The given role doesn't exist. */
    InvalidRole,
    /** (Unban only) The target isn't banned. */
    NotBanned,
    /** The request couldn't be processed (e.g. the server couldn't be
        reached). */
    InternalError,
    Count
};

/**
 * Returns true if the given action is a real action value.
 *
 * Actions are received over the network, so they must be checked before use.
 */
inline constexpr bool
    isValidAccountModerationAction(AccountModerationAction action)
{
    return static_cast<Uint8>(action)
           < static_cast<Uint8>(AccountModerationAction::Count);
}

/**
 * Returns true if the given action is a real action value.
 *
 * Actions are received over the network, so they must be checked before use.
 */
inline constexpr bool isValidWorldModerationAction(WorldModerationAction action)
{
    return static_cast<Uint8>(action)
           < static_cast<Uint8>(WorldModerationAction::Count);
}

} // End namespace AM
