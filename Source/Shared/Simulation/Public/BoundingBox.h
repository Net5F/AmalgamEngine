#pragma once

#include "TileExtent.h"
#include "Position.h"
#include "Log.h"
#include <cmath>

namespace AM
{
/**
 * Represents a set of axis-aligned 3D bounds, forming a box.
 */
struct BoundingBox {
public:
    float minX{0};
    float maxX{0};

    float minY{0};
    float maxY{0};

    float minZ{0};
    float maxZ{0};

    bool operator==(const BoundingBox& other)
    {
        return (minX == other.minX) && (maxX == other.maxX)
               && (minY == other.minY) && (maxY == other.maxY)
               && (minZ == other.minZ) && (maxZ == other.maxZ);
    }

    float getXLength() const { return (maxX - minX); }

    float getYLength() const { return (maxY - minY); }

    float getZLength() const { return (maxZ - minZ); }

    /**
     * Returns a position at the minimum point of this bounding box.
     */
    Position getMinPosition() const
    {
        return {minX, minY, minZ};
    }

    /**
     * Returns a position at the maximum point of this bounding box.
     */
    Position getMaxPosition() const
    {
        return {maxX, maxY, maxZ};
    }

    /**
     * Returns a position at the center of this bounding box.
     *
     * Note: This center position is different than an entity's Position
     *       component. This one is centered in all axis, while an entity's
     *       Position is centered in the X/Y but is at the minimum Z.
     */
    Position get3dCenter() const
    {
        Position centerPosition{};
        centerPosition.x = minX + ((maxX - minX) / 2);
        centerPosition.y = minY + ((maxY - minY) / 2);
        centerPosition.z = minZ + ((maxZ - minZ) / 2);

        return centerPosition;
    }

    /**
     * Returns true if this box intersects the given other bounding box.
     */
    bool intersects(const BoundingBox& other) const
    {
        return ((minX < other.maxX) && (maxX > other.minX) &&
            (minY < other.maxY) && (maxY > other.minY) &&
            (minZ < other.maxZ) && (maxZ > other.minZ));
    }

    /**
     * Returns true if this box intersects the given cylinder.
     *
     * Note: This treats the given data as a cylinder with infinite length
     *       along the Z axis. If we want to treat it as a sphere, we can
     *       change it.
     * Note: Shared edges are considered to be intersecting.
     *
     * Reference: https://stackoverflow.com/a/402010/4258629
     */
    bool intersects(const Position& cylinderCenter, unsigned int radius) const
    {
        Position boxCenter{get3dCenter()};
        float xLength{getXLength()};
        float yLength{getYLength()};

        // Get the X and Y distances between the centers.
        float circleDistanceX{std::abs(cylinderCenter.x - boxCenter.x)};
        float circleDistanceY{std::abs(cylinderCenter.y - boxCenter.y)};

        // If the circle is far enough away that no intersection is possible,
        // return false.
        if (circleDistanceX > ((xLength / 2) + radius)) {
            return false;
        }
        if (circleDistanceY > ((yLength / 2) + radius)) {
            return false;
        }

        // If the circle is close enough that an intersection is guaranteed,
        // return true.
        if (circleDistanceX <= (xLength / 2)) {
            return true;
        }
        if (circleDistanceY <= (yLength / 2)) {
            return true;
        }

        // Calculate the distance from the center of the circle to the corner
        // of the box.
        float xDif{circleDistanceX - (xLength / 2)};
        float yDif{circleDistanceY - (yLength / 2)};
        float cornerDistanceSquared{(xDif * xDif) + (yDif * yDif)};

        // If the distance is less than the radius, return true.
        return (cornerDistanceSquared <= (radius * radius));
    }

    /**
     * Returns true if this box intersects the given tile extent.
     *
     * Note: This treats the tile extent as having infinite length along the
     *       Z axis.
     * Note: Shared edges are considered to be intersecting.
     */
    bool intersects(const TileExtent& tileExtent) const
    {
        const int TILE_WORLD_WIDTH{
            static_cast<int>(SharedConfig::TILE_WORLD_WIDTH)};

        float tileMinX{static_cast<float>(tileExtent.x * TILE_WORLD_WIDTH)};
        float tileMaxX{static_cast<float>((tileExtent.x + tileExtent.xLength)
                                          * TILE_WORLD_WIDTH)};
        float tileMinY{static_cast<float>(tileExtent.y) * TILE_WORLD_WIDTH};
        float tileMaxY{static_cast<float>((tileExtent.y + tileExtent.yLength)
                                          * TILE_WORLD_WIDTH)};

        return ((maxX >= tileMinX) && (tileMaxX >= minX) && (maxY >= tileMinY)
                && (tileMaxY >= minY));
    }

    /**
     * Returns the smallest tile extent that contains this bounding box.
     *
     * Note: The Z-axis is ignored in this conversion, as TileExtent is 2D.
     */
    TileExtent asTileExtent() const
    {
        TileExtent tileExtent{};
        const float tileWorldWidth{
            static_cast<float>(SharedConfig::TILE_WORLD_WIDTH)};

        tileExtent.x = static_cast<int>(std::floor(minX / tileWorldWidth));
        tileExtent.y = static_cast<int>(std::floor(minY / tileWorldWidth));
        tileExtent.xLength = (static_cast<int>(std::ceil(maxX / tileWorldWidth)) - tileExtent.x);
        tileExtent.yLength = (static_cast<int>(std::ceil(maxY / tileWorldWidth)) - tileExtent.y);

        return tileExtent;
    }
};

} // End namespace AM
