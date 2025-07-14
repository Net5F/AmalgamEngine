#pragma once

#include "CastableID.h"
#include "CastableProjectData.h"
#include "CastableRequirementType.h"
#include "EntityGraphicType.h"
#include "VisualEffect.h"
#include "AVEntity.h"
#include "Cylinder.h"
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
    /** This Castable's unique ID. */
    CastableID castableID{};

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
    float range{0};

    /** If targetToolType == Circle, this is the radius of the circle. */
    float radius{0};

    /** How long this castable takes to cast. */
    float castTime{0};

    /** How long the caster must wait to cast this castable again. */
    float cooldownTime{0};

    /** If true, casting this castable will trigger the global cooldown (see 
        SharedConfig::CAST_GLOBAL_COOLDOWN_S). */
    bool triggersGCD{true};

    /** If true, this cast will be queued by the server and processed on the 
        tick when the client cast it (clients are in the future). If false, it 
        will be processed as soon as the server receives it.
        Most castables will want this to be true. False is useful for things 
        that should be instant and don't affect the sim, e.g. Examine, Talk. */
    bool isTickSynchronized{true};

    /** Project-specific data. */
    CastableProjectData projectData{};

    struct Requirement {
        CastableRequirementType type{};
        int value{};
    };
    /** The requirements to cast this castable. */
    std::vector<Requirement> requirements{};

    /** The graphic to loop while this castable is being cast. */
    EntityGraphicType castingGraphicType{EntityGraphicType::NotSet};

    // TODO: castingSoundType

    /** The graphic to play when this castable is successfully cast. */
    EntityGraphicType castCompleteGraphicType{EntityGraphicType::NotSet};

    // TODO: castCompleteSoundType

    /** The client-only visual effects to play when this cast is successful. */
    std::vector<VisualEffect> castCompleteVisualEffects{};

    /** The client-only audio effects to play when this cast is successful. */
    //std::vector<AudioEffect> castCompleteAudioEffects{};

    /** The client-only A/V entities to spawn when this cast is successful. */
    std::vector<AVEntity> castCompleteAVEntities{};

    /**
     * Returns a target cylinder centered on the given position along the X/Y 
     * axes, and sitting on top of it along the Z axis.
     *
     * Only valid if targetToolType == Circle and radius is set.
     */
    Cylinder getTargetCylinder(const Vector3& position) const;

    /**
     * Returns true if this castable has any graphics or effects.
     */
    bool hasVisuals() const;
};

} // namespace AM
