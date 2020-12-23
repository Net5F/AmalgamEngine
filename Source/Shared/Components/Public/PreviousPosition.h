#pragma once

namespace AM
{
/**
 * Represents an entity's previous position.
 * Used for lerping during things like movement and rendering.
 */
struct PreviousPosition {
public:
    //--------------------------------------------------------------------------
    // Non-replicated data
    //--------------------------------------------------------------------------
    /** Previous position, used for lerping. */
    float x{0};
    float y{0};
    float z{0};
};

} // namespace AM
