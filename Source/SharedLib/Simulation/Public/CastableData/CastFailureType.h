#pragma once

#include <SDL_stdinc.h>

namespace AM
{

/**
 * All of the ways in which a Castable cast may fail.
 */
enum class CastFailureType : Uint8 {
    None,
    /** A client ID was provided, but the specified client does not exist. */
    InvalidClient,
    /** The given castable ID doesn't exist. */
    InvalidCastable,
    /** The caster entity is already casting something. */
    AlreadyCasting,
    /** The requested cast is on cooldown, or the GCD is active. */
    OnCooldown,
    /** The target entity or position is out of range of the caster. */
    OutOfRange,
    /** The target item does not exist. */
    InvalidItem,
    /** The target item or entity doesn't support the requested interaction 
        type. */
    InteractionNotSupported,
    /** The caster entity doesn't exist. */
    InvalidCasterEntity,
    /** The target entity doesn't exist. */
    InvalidTargetEntity,
    /** The target position is outside of the tile map. */
    InvalidTargetPosition,
    /** The project's validateCast() call returned false. */
    ProjectValidationFailed,
    /** The caster entity moved. */
    Movement,
};

/**
 * Returns a user-facing failure string for the given failure type.
 */
inline const char* getCastFailureString(CastFailureType failureType)
{
    // Note: We only add strings for failure types that the user can do 
    //       something about. Internal failures due to e.g. bugs shouldn't be 
    //       sent to the user.
    switch (failureType) {
        case CastFailureType::AlreadyCasting:
            return "A cast is already underway.";
        // Note: We don't add a failure message for OnCooldown because there 
        //       should already be something in the UI to show cooldowns.
        //case CastFailureType::OnCooldown:
        case CastFailureType::OutOfRange:
            return "Out of range.";
        case CastFailureType::InvalidTargetEntity:
            return "You have no target.";
        case CastFailureType::Movement:
            return "Can't do that while moving.";
        default:
            break;
    }

    return "";
}

} // namespace AM
