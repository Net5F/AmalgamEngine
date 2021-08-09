#include "ClientMovementHelpers.h"
#include "Sprite.h"
#include "Position.h"

namespace AM
{
namespace Client
{
void ClientMovementHelpers::moveSpriteWorldBounds(Position& position,
                                                  Sprite& sprite)
{
    // Move the sprite's world bounds to the given position.
    BoundingBox& modelBox = sprite.modelBounds;
    BoundingBox& worldBox = sprite.worldBounds;

    worldBox.minX = position.x + modelBox.minX;
    worldBox.maxX = position.x + modelBox.maxX;
    worldBox.minY = position.y + modelBox.minY;
    worldBox.maxY = position.y + modelBox.maxY;
    worldBox.minZ = position.z + modelBox.minZ;
    worldBox.maxZ = position.z + modelBox.maxZ;
}

} // End namespace Client
} // End namespace AM
