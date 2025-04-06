#include "CastCooldown.h"

namespace AM
{

bool CastCooldown::isCastOnCooldown(CastableID castableID, Uint32 currentTick)
{
    Uint32 tickDiff{currentTick - lastUpdateTick};

    // Update the GCD.
    if (gcdTicksRemaining > 0) {
        if (gcdTicksRemaining <= tickDiff) {
            gcdTicksRemaining = 0;
        }
        else {
            // GCD is still active. Return early.
            gcdTicksRemaining -= tickDiff;
            return true;
        }
    }

    // Update the rest of the cooldowns.
    // Note: This uses the erase-remove idiom, similar to std::remove.
    bool castableFound{false};
    auto first{cooldowns.begin()};
    for (auto it{cooldowns.begin()}; it != cooldowns.end(); ++it) {
        if (it->ticksRemaining > tickDiff) {
            // This cooldown hasn't expired. Update its time.
            it->ticksRemaining -= tickDiff;

            // If this is the desired castable, mark it as found.
            if (it->castableID == castableID) {
                castableFound = true;
            }

            // Move this cooldown in front of any expired cooldowns.
            *(first++) = std::move(*it);
        }
    }

    // Erase all of the expired cooldowns.
    cooldowns.erase(first, cooldowns.end());

    lastUpdateTick = currentTick;

    return castableFound;
}

void CastCooldown::update(Uint32 newTick)
{
    Uint32 tickDiff{newTick - lastUpdateTick};

    // Update the GCD.
    if (gcdTicksRemaining <= tickDiff) {
        gcdTicksRemaining = 0;
    }
    else {
        gcdTicksRemaining -= tickDiff;
    }

    // Update the rest of the cooldowns.
    // Note: This uses the erase-remove idiom, similar to std::remove.
    auto first{cooldowns.begin()};
    for (auto it{cooldowns.begin()}; it != cooldowns.end(); ++it) {
        if (it->ticksRemaining > tickDiff) {
            // This cooldown hasn't expired. Update its time.
            it->ticksRemaining -= tickDiff;

            // Move this cooldown in front of any expired cooldowns.
            *(first++) = std::move(*it);
        }
    }

    // Erase all of the expired cooldowns.
    cooldowns.erase(first, cooldowns.end());

    // Track that we updated to the given tick.
    lastUpdateTick = newTick;
}

void CastCooldown::initAfterLoad(Uint32 lastSavedTick, Uint32 currentTick)
{
    // Update to account for the time between the last update and when this 
    // component was saved.
    update(lastSavedTick);

    // Since we've accounted for all the time pre-save, we can now start 
    // tracking the current session.
    lastUpdateTick = currentTick;
}

} // End namespace AM
