#include "Transforms.h"
#include "Position.h"
#include "ScreenPoint.h"
#include "Camera.h"
#include "BoundingBox.h"
#include "SharedConfig.h"
#include "Log.h"
#include <cmath>

namespace AM
{
ScreenPoint Transforms::worldToScreen(const Position& position,
                                      float zoomFactor)
{
    // Calc the scaling factor going from world tiles to screen tiles.
    static const float TILE_WIDTH_SCALE
        = static_cast<float>(SharedConfig::TILE_SCREEN_WIDTH)
          / SharedConfig::TILE_WORLD_WIDTH;
    static const float TILE_HEIGHT_SCALE
        = static_cast<float>(SharedConfig::TILE_SCREEN_HEIGHT)
          / SharedConfig::TILE_WORLD_WIDTH;

    // Convert cartesian world point to isometric screen point.
    float screenX{(position.x - position.y) * (TILE_WIDTH_SCALE / 2.f)};
    float screenY{(position.x + position.y) * (TILE_HEIGHT_SCALE / 2.f)};

    // The Z coordinate scaling is independent of X/Y and only affects the
    // screen's Y axis. Scale and apply it.
    screenY -= (position.z * SharedConfig::Z_SCREEN_SCALE);

    // Apply the camera zoom.
    screenX *= zoomFactor;
    screenY *= zoomFactor;

    return {screenX, screenY};
}

float Transforms::worldZToScreenY(float zCoord, float zoomFactor)
{
    return zCoord * zoomFactor * SharedConfig::Z_SCREEN_SCALE;
}

Position Transforms::screenToWorld(const ScreenPoint& screenPoint,
                                   const Camera& camera)
{
    // Remove the camera adjustment.
    ScreenPoint absolutePoint;
    absolutePoint.x = screenPoint.x + camera.extent.x;
    absolutePoint.y = screenPoint.y + camera.extent.y;

    // Remove the camera zoom.
    float x{absolutePoint.x / camera.zoomFactor};
    float y{absolutePoint.y / camera.zoomFactor};

    // Calc the scaling factor going from screen tiles to world tiles.
    static const float TILE_WIDTH_SCALE
        = static_cast<float>(SharedConfig::TILE_WORLD_WIDTH)
          / SharedConfig::TILE_SCREEN_WIDTH;
    static const float TILE_HEIGHT_SCALE
        = static_cast<float>(SharedConfig::TILE_WORLD_WIDTH)
          / SharedConfig::TILE_SCREEN_HEIGHT;

    // Calc the world position.
    float worldX{((2.f * y) + x) * TILE_WIDTH_SCALE};
    float worldY{((2.f * y) - x) * TILE_HEIGHT_SCALE / 2.f};

    // TODO: Figure out how to handle Z.
    return {worldX, worldY, 0};
}

float Transforms::screenYToWorldZ(float yCoord, float zoomFactor)
{
    // Calc the scaling factor going from screen Y units to world Z units.
    static const float Z_WORLD_SCALE = 1 / SharedConfig::Z_SCREEN_SCALE;

    return yCoord * zoomFactor * Z_WORLD_SCALE;
}

TilePosition Transforms::screenToTile(const ScreenPoint& screenPoint,
                                      const Camera& camera)
{
    // Convert to world space.
    Position worldPos = screenToWorld(screenPoint, camera);

    // Convert to tile index.
    return worldPos.asTilePosition();
}

BoundingBox Transforms::modelToWorld(const BoundingBox& modelBounds,
                                     const Position& position)
{
    // Place the model-space bounding box at the given position.
    BoundingBox movedBox{};
    movedBox.minX = position.x + modelBounds.minX;
    movedBox.maxX = position.x + modelBounds.maxX;
    movedBox.minY = position.y + modelBounds.minY;
    movedBox.maxY = position.y + modelBounds.maxY;
    movedBox.minZ = position.z + modelBounds.minZ;
    movedBox.maxZ = position.z + modelBounds.maxZ;

    return movedBox;
}

BoundingBox Transforms::modelToWorldCentered(const BoundingBox& modelBounds,
                                             const Position& position)
{
    // Place the model-space bounding box at the given position, shifted back
    // by a half tile to center it.
    // Note: This assumes that the sprite is 1 tile large. When we add support
    //       for other sizes, this will need to be updated.
    BoundingBox movedBox{};
    movedBox.minX
        = position.x + modelBounds.minX - (SharedConfig::TILE_WORLD_WIDTH / 2);
    movedBox.minY
        = position.y + modelBounds.minY - (SharedConfig::TILE_WORLD_WIDTH / 2);
    movedBox.maxX = movedBox.minX + modelBounds.getXLength();
    movedBox.maxY = movedBox.minY + modelBounds.getYLength();

    // Place the bottom of the box flush with the given position.
    // (e.g. if this is a character's position, the bottom of the box is at
    // their feet).
    movedBox.minZ = position.z + modelBounds.minZ;
    movedBox.maxZ = position.z + modelBounds.maxZ;

    return movedBox;
}

} // End namespace AM
