#pragma once

#include "EntityGraphicType.h"
#include "CastableRequirementType.h"
#include "VisualEffect.h"
#include "AVEntity.h"
#include <functional>
#include <vector>
#include <variant>

namespace AM
{

/**
 * An abstraction that groups interactions and spells into a single concept.
 *
 * Casts are defined in a few stages:
 *     1. Pre-cast
 *         Before the cast has started. Check the requirements, make sure the 
 *         entity can cast it.
 *     2. Casting
 *         The cast has begun. Set the caster to a casting graphic, send a 
 *         request to the server.
 *     3. (Predicted) Cast complete
 *         The cast has finished on the client. If the Castable is set to be 
 *         predicted, play any resulting A/V effects.
 *     4. Cast complete
 *         The cast has finished on the server. Process any world state 
 *         changes. If the Castable is set to non-predicted, play any resulting
 *         A/V effects.
 */
struct Castable {
    enum class TargetToolType {
        /** No extra targeting required (likely self-cast). */
        None,
        /** Require another entity be targeted. */
        Entity,
        /** Place a circle on the ground. */
        Circle 
    };

    /** The type of UI tool to present to the user when casting.
        Determines what data will be packed into the "cast request" message.
        The user's current targeted entity will always be provided (if there 
        is one), but other data requires extra UI steps (e.g. placing an AoE  
        circle). */
    TargetToolType targetToolType{};

    /** How far away the target can be. */
    float range{};

    /** If targetToolType == Cylinder, this is the radius of the circle. */
    float radius{};

    /** How long this interaction takes to cast. */
    float castTime{};

    /** How long the caster must wait to cast this interaction again. */
    float cooldownTime{};

    struct Requirement {
        CastableRequirementType type{};
        int value{};
    };

    /** The requirements to cast this castable. */
    std::vector<Requirement> requirements{};

    /** The graphic to loop while this interaction is being cast. */
    EntityGraphicType castingGraphicType{};

    // TODO: castingSoundType

    /** The graphic to play when this interaction is successfully cast. */
    EntityGraphicType castCompleteGraphicType{};

    // TODO: castCompleteSoundType

    /** If true, the client will play the "cast complete" graphics when the 
        predicted cast (which starts as soon as the player input is received) 
        completes. If false, the graphics won't be played until we receive a 
        confirmation from the server that the cast was successful. */
    bool predictCastComplete{};

    /** The client-only visual effects to show when this cast is successful. */
    std::vector<VisualEffect> castCompleteVisualEffects{};

    /** The client-only A/V entities to spawn when this cast is successful. */
    std::vector<AVEntity> castCompleteAVEntities{};
};

} // namespace AM
