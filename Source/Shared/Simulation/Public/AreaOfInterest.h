#pragma once

#include "Position.h"

namespace AM
{
/**
 * An area of interest in world space--that is, the area that entities must be
 * within in order to be replicated to a given client.
 *
 * Note: Currently 2D as a rough pass since this is likely to change when we
 *       implement collision. This might be replaced with a collision system's
 *       primitives and operations if it's convenient.
 */
struct AreaOfInterest {
public:
    /** The width of this AoI, projected along the X axis from the origin. */
    float width{0};

    /** The height of this AoI, projected along the Y axis from the origin. */
    float height{0};

    /** The origin of this area of interest. Should be set through setCenter. */
    Position origin{};

    /**
     * Centers this AoI on the given position.
     */
    void setCenter(const Position& position)
    {
        origin.x = (position.x - (width / 2));
        origin.y = (position.y - (height / 2));
    }

    /**
     * @return true if the given position is within this AoI, else false.
     */
    bool contains(const Position& position)
    {
        return (position.x >= origin.x) && (position.y >= origin.y)
               && (position.x <= (origin.x + width))
               && (position.y <= (origin.y + height));
    }
};

} // End namespace AM
