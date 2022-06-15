#pragma once

#include "Position.h"

namespace AM
{
/**
 * Represents an entity's previous position.
 * Used for lerping during things like movement and rendering.
 */
struct PreviousPosition : Position {
    /** Used to tell if this position has been set or not. */
    bool isInitialized{false};

    PreviousPosition& operator=(const Position& other)
    {
        x = other.x;
        y = other.y;
        z = other.z;

        return *this;
    }
};

} // namespace AM
