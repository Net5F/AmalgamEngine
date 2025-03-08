#pragma once

#include <SDL_stdinc.h>

namespace AM
{

// TODO: Is this what we want? Who gets a CastingState? Would be nice to 
//       remove when not in use, but would that be expensive?
//       Options:
//         Every entity has a CastingState, it has a bool isCasting to check.
//         CastingState is added only if an entity is casting, use any_of<> 
//         to check.
/**
 * 
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
