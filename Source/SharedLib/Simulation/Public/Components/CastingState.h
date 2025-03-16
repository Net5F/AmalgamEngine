#pragma once

#include <SDL_stdinc.h>

namespace AM
{

/**
 * Tracks whether an entity is currently casting a Castable.
 * 
 * This component will only be present on an entity if a cast is currently 
 * ongoing. It gets removed when the cast ends.
 *
 * Note: We manually replicate this component. If we auto-replicated, all 
 *       destructions would be replicated. By handling it manually, we can get 
 *       away with only sending CastStarted and CastCanceled (a successful cast 
 *       can be assumed when there's no cancelation before endTick).
 */
struct CastingState {
    enum class CastableType : Uint8 {
        ItemInteraction,
        EntityInteraction,
        Spell
    };

    /** The type of castable that's being cast. */
    CastableType castableType{};

    /** The value of the enum associated with castableType, either 
        ItemInteractionType, EntityInteractionType, or SpellType. */
    Uint8 castableEnumValue{};

    /** The tick that this cast will finish on. */
    Uint32 endTick{};
};

} // namespace AM
