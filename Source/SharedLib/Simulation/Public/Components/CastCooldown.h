#pragma once

#include "CastableID.h"
#include <vector>

namespace AM
{

/**
 * Tracks cooldowns for any Castables that the entity recently cast.
 *
 * This component isn't replicated on every update like others (the constant 
 * timer updates would cause tons of traffic). Instead, we only send an update 
 * when something is added to it.
 */
struct CastCooldown {
    /** Used as a "we should never hit this" cap on the number of cooldowns. */
    static constexpr std::size_t MAX_COOLDOWNS{100};

    struct Cooldown {
        // The Castable that is on cooldown.
        CastableID castableID{};

        /** The remaining ticks for this cooldown. */
        Uint32 ticksRemaining{0};
    };

    /** The latest tick when this component was updated. */
    Uint32 lastUpdateTick{};

    /** The remaining ticks for the "global cooldown", a cooldown that occurs 
        after every cast. See SharedConfig::CAST_GLOBAL_COOLDOWN_S. */
    Uint32 gcdTicksRemaining{0};

    /** The remaining ticks for each cooldown. */
    std::vector<Cooldown> cooldowns{};

    /**
     * Returns true if the global cooldown is active, or the given cast is on 
     * cooldown.
     * 
     * This function has a side effect of updating all of this component's 
     * cooldown times and removing expired cooldowns. We do this because it's 
     * efficient: we need to update the desired cooldown (if present) anyway 
     * to tell if it has expired, so doing the rest of the updates is 
     * relatively low-cost.
     */
    bool isCastOnCooldown(CastableID castableID, Uint32 currentTick);

    /**
     * Updates all cooldowns to the given tick.
     */
    void update(Uint32 newTick);

    /**
     * Initializes this component after being loaded from the database.
     *
     * Since cooldowns are lazy-updated, when they're loaded they may not have 
     * accounted for the time between when they were last updated and when they
     * were saved.
     * 
     * @param lastSavedTick The tick when the data was saved to the DB.
     * @param currentTick The current tick.
     */
    void initAfterLoad(Uint32 lastSavedTick, Uint32 currentTick);
};

template<typename S>
void serialize(S& serializer, CastCooldown::Cooldown& cooldown)
{
    serializer.object(cooldown.castableID);
    serializer.value4b(cooldown.ticksRemaining);
}

template<typename S>
void serialize(S& serializer, CastCooldown& castCooldown)
{
    serializer.value4b(castCooldown.lastUpdateTick);
    serializer.value4b(castCooldown.gcdTicksRemaining);
    serializer.container(castCooldown.cooldowns, CastCooldown::MAX_COOLDOWNS);
}

} // namespace AM
