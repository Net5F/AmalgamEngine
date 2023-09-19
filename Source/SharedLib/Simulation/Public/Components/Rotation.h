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
 *
 * Note: This is specifically a rotation component that will be attached to an 
 *       entity. When doing 3D math, we use other structs (Position, Ray, etc).
 */
struct Rotation {
    /**
     * The 8 directions that rotation is locked to.
     */
    enum Direction : Uint8 {
        South,
        SouthWest,
        West,
        NorthWest,
        North,
        NorthEast,
        East,
        SouthEast,
        Count,
        None, /*!< No inputs, or inputs are canceling eachother out. */
    };

    /** The current direction that the entity is facing. */
    Direction direction{Direction::South};
};

template<typename S>
void serialize(S& serializer, Rotation& rotation)
{
    serializer.value1b(rotation.direction);
}

} // namespace AM
