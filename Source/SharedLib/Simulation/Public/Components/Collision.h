#pragma once

#include "BoundingBox.h"

namespace AM
{
/**
 * Represents an entity's collision bounds (a.k.a collision box, or just
 * collision).
 *
 * Entities use this component to define a consistent collision box. This 
 * allows us to change their animation without worrying about re-calculating 
 * their collision.
 * Tiles just use their Sprite's model bounds to define their collision.
 */
struct Collision {
    /** Model-space bounding box. Defines the entity's 3D volume.
        Note: When an entity is constructed, we set this to match the default 
              sprite's model bounds. This makes it easy for project devs to 
              define the default entity collision. If the project dev wants 
              different default collision for different entities, they can 
              handle it in a project system. */
    BoundingBox modelBounds{0, 0, 0, 0, 0, 0};

    /** World-space bounding box. This is modelBounds, moved to the entity's
        position in the world. */
    BoundingBox worldBounds{0, 0, 0, 0, 0, 0};
};

} // End namespace AM
