#pragma once

#include "Log.h"
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

namespace DisplayStrings
{
inline std::string get(Rotation::Direction direction)
{
    switch (direction) {
        case Rotation::Direction::South:
            return "S";
        case Rotation::Direction::SouthWest:
            return "SW";
        case Rotation::Direction::West:
            return "W";
        case Rotation::Direction::NorthWest:
            return "NW";
        case Rotation::Direction::North:
            return "N";
        case Rotation::Direction::NorthEast:
            return "NE";
        case Rotation::Direction::East:
            return "E";
        case Rotation::Direction::SouthEast:
            return "SE";
        default:
            break;
    }

    LOG_ERROR("Tried to get display string for unknown direction.");
    return "?";
}

} // namespace DisplayStrings
} // namespace AM
