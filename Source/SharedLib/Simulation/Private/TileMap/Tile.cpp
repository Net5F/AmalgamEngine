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
        std::optional<GraphicRef> graphic{wallLayer.getGraphic()};
        if (graphic && graphic->getCollisionEnabled()) {
            collisionBoxes.push_back(
                calcWorldBoundsForGraphic(tileX, tileY, *graphic));
        }
    }

    // Add all of this tile's objects.
    for (const ObjectTileLayer& objectLayer : getObjects()) {
        std::optional<GraphicRef> graphic{objectLayer.getGraphic()};
        if (graphic && graphic->getCollisionEnabled()) {
            collisionBoxes.push_back(
                calcWorldBoundsForGraphic(tileX, tileY, *graphic));
        }
    }
}

BoundingBox Tile::calcWorldBoundsForGraphic(int tileX, int tileY,
                                            const GraphicRef& graphic)
{
    Position tilePosition{
        static_cast<float>(tileX * SharedConfig::TILE_WORLD_WIDTH),
        static_cast<float>(tileY * SharedConfig::TILE_WORLD_WIDTH), 0};
    return Transforms::modelToWorld(graphic.getModelBounds(), tilePosition);
}

} // End namespace AM
