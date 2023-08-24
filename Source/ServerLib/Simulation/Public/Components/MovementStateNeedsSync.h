#pragma once

namespace AM
{

/**
 * Used to flag that an entity's current movement state needs to be
 * re-synced with nearby clients.
 *
 * Reasons for needing to re-sync movement state include:
 *   1. The entity's inputs changed (the user pressed or released a key).
 *   2. We had to drop a movement update request message from a client
 *      (in such a case, we zero-out their input state so they don't run off
 *      a cliff).
 *   3. The entity was teleported.
 */
struct MovementStateNeedsSync {
};

} // End namespace AM
