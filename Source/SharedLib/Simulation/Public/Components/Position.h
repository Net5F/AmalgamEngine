#pragma once

#include "Vector3.h"

namespace AM
{
/**
 * Represents a position in the world.
 *
 * Note: An entity's position component refers to a centered point under its 
 *       feet (equivalent to BoundingBox::getBottomCenterPoint()). Do not 
 *       confuse this for the 3D center of a bounding box.
 *
 * All entities possess a Position component.
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
