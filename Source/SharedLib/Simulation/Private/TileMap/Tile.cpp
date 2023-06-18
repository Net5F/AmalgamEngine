#include "Tile.h"
#include "Sprite.h"
#include "Transforms.h"
#include "SharedConfig.h"

namespace AM
{

Tile::Tile()
: collisionBoxes{}
, layers{std::make_unique<Layers>()}
{
}

const std::vector<BoundingBox>& Tile::getCollisionBoxes() const
{
    return collisionBoxes;
}

const FloorTileLayer& Tile::getFloor() const
{
    return layers->floor;
}

FloorTileLayer& Tile::getFloor()
{
    return layers->floor;
}

const std::vector<FloorCoveringTileLayer>& Tile::getFloorCoverings() const
{
    return layers->floorCoverings;
}

std::vector<FloorCoveringTileLayer>& Tile::getFloorCoverings()
{
    return layers->floorCoverings;
}

const std::array<WallTileLayer, 2>& Tile::getWalls() const
{
    return layers->walls;
}

std::array<WallTileLayer, 2>& Tile::getWalls()
{
    return layers->walls;
}

const std::vector<ObjectTileLayer>& Tile::getObjects() const
{
    return layers->objects;
}

std::vector<ObjectTileLayer>& Tile::getObjects()
{
    return layers->objects;
}

bool Tile::hasWestWall() const
{
    return layers->walls[0].wallType == Wall::Type::West;
}

bool Tile::hasNorthWall() const
{
    return (layers->walls[1].wallType == Wall::Type::North)
           || (layers->walls[1].wallType == Wall::Type::NorthEastGapFill);
}

void Tile::rebuildCollision(int tileX, int tileY)
{
    // Clear out the old collision boxes.
    collisionBoxes.clear();

    // Add all of this tile's walls.
    for (const WallTileLayer& wallLayer : getWalls()) {
        // Note: Walls may be empty, in which case sprite == nullptr.
        const Sprite* sprite{wallLayer.getSprite()};
        if ((sprite != nullptr) && sprite->collisionEnabled) {
            collisionBoxes.push_back(
                calcWorldBoundsForSprite(tileX, tileY, sprite));
        }
    }

    // Add all of this tile's objects.
    for (const ObjectTileLayer& objectLayer : getObjects()) {
        const Sprite* sprite{objectLayer.getSprite()};
        if (sprite->collisionEnabled) {
            collisionBoxes.push_back(
                calcWorldBoundsForSprite(tileX, tileY, sprite));
        }
    }
}

BoundingBox Tile::calcWorldBoundsForSprite(int tileX, int tileY, const Sprite* sprite)
{
    Position tilePosition{
        static_cast<float>(tileX * SharedConfig::TILE_WORLD_WIDTH),
        static_cast<float>(tileY * SharedConfig::TILE_WORLD_WIDTH), 0};
    return Transforms::modelToWorld(sprite->modelBounds, tilePosition);
}

} // End namespace AM
