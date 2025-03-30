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
    ProjectValidationFailed
};

} // namespace AM
