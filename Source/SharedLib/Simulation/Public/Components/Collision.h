#pragma once

#include "BoundingBox.h"

namespace AM
{
/**
 * Represents an entity's collision bounds (a.k.a collision box, or just 
 * collision).
 *
 * Dynamic entities (ones that move, such as players) use this component to 
 * define a consistent collision box. This allows us to change their animation 
 * without worrying about re-calculating their collision.
 * Tiles and static entities just use their Sprite's bounding box.
 */
struct Collision {
public:
    /** Model-space bounding box. Defines the entity's 3D volume. */
    BoundingBox modelBounds{0, 0, 0, 0, 0, 0};

    /** World-space bounding box. This is modelBounds, moved to the entity's 
        position in the world. */
    BoundingBox worldBounds{0, 0, 0, 0, 0, 0};
};

} // End namespace AM
