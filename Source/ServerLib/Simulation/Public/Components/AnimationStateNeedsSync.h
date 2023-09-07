#pragma once

namespace AM
{

/**
 * Used to flag that an entity's current animation state needs to be re-synced 
 * with nearby clients.
 *
 * Reasons for needing to re-sync sprite state include:
 *   1. A client changed the entity's sprite using build mode.
 *   2. The entity had a simple state change (e.g. opening a door).
 *   3. The entity was transmogrified by some system.
 */
struct AnimationStateNeedsSync {
};

} // End namespace AM
