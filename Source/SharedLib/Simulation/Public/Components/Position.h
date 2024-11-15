#pragma once

#include "Vector3.h"

namespace AM
{
/**
 * Represents a position in the world.
 *
 * All entities possess a Position component.
 *
 * Note: An entity's graphics will be aligned with its position (specifically, 
 *       the graphic's alignment anchor will be centered on the position).
 */
struct Position : Vector3 {
    Position operator=(const Vector3& vector)
    {
        x = vector.x;
        y = vector.y;
        z = vector.z;
        return *this;
    }
};

template<typename S>
void serialize(S& serializer, Position& position)
{
    serializer.value4b(position.x);
    serializer.value4b(position.y);
    serializer.value4b(position.z);
}

} // namespace AM
