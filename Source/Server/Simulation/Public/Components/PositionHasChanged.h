#pragma once

namespace AM
{
/**
 * Used to flag that an entity's position has changed on this tick.
 *
 * Note: This is currently server-only since we don't have a use for it in
 *       the client, but we can move it to Shared if a use pops up.
 */
struct PositionHasChanged {
};

} // End namespace AM
