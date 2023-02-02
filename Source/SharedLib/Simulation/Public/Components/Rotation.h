#pragma once

#include <SDL_stdinc.h>

namespace AM
{

/**
 * Represents an entity's rotation about the z-axis, i.e. yaw, or "the
 * direction the entity is facing".
 *
 * Due to being a sprite-based engine, rotation about other axes is not viable,
 * and rotation is locked to 8 directions.
 */
struct Rotation {
public:
    /** The 8 directions that rotation is locked to. */
    enum Direction : Sint8 {
        SouthWest = -4,
        South = -3,
        SouthEast = -2,
        West = -1,
        None = 0, /*!< No inputs, or inputs are canceling eachother out. */
        East = 1,
        NorthWest = 2,
        North = 3,
        NorthEast = 4
    };

    //--------------------------------------------------------------------------
    // Replicated data
    //--------------------------------------------------------------------------
    /** The current direction that the entity is facing. */
    Direction direction{Direction::South};
};

template<typename S>
void serialize(S& serializer, Rotation& rotation)
{
    serializer.value1b(rotation.direction);
}

} // namespace AM
