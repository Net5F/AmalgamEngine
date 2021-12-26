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

    float getXLength()
    {
        return (maxX - minX);
    }

    float getYLength()
    {
        return (maxY - minY);
    }

    float getZLength()
    {
        return (maxZ - minZ);
    }

    /**
     * Returns a position at the center of this bounding box.
     */
    Position getCenterPosition()
    {
        Position centerPosition{};
        centerPosition.x = minX + ((maxX - minX) / 2);
        centerPosition.y = minY + ((maxY - minY) / 2);
        centerPosition.z = minZ + ((maxZ - minZ) / 2);

        return centerPosition;
    }

    /**
     * Returns true if this box intersects the given cylinder.
     *
     * Note: This treats the given data as a cylinder with infinite length
     *       along the Z axis. If we want to treat it as a sphere, we can
     *       change it.
     *
     * Reference: https://stackoverflow.com/a/402010/4258629
     */
    bool intersects(const Position& cylinderCenter, unsigned int radius)
    {
        Position boxCenter{getCenterPosition()};
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
     */
    bool intersects(const TileExtent& tileExtent)
    {
        float tileMinX{static_cast<float>(tileExtent.x)};
        float tileMaxX{static_cast<float>(tileExtent.x + tileExtent.xLength)};
        float tileMinY{static_cast<float>(tileExtent.y)};
        float tileMaxY{static_cast<float>(tileExtent.y + tileExtent.yLength)};

        return ((maxX >= tileMinX) && (tileMaxX >= minX)
               && (maxY >= tileMinY) && (tileMaxY >= minY));
    }

    /**
     * Returns the smallest tile extent that contains this bounding box.
     *
     * Note: The Z-axis is ignored in this conversion, as TileExtent is 2D.
     */
    TileExtent asTileExtent() const
    {
        TileExtent tileExtent{};
        const float tileWorldWidth{static_cast<float>(SharedConfig::TILE_WORLD_WIDTH)};
        tileExtent.x = static_cast<int>(minX / tileWorldWidth);
        tileExtent.y = static_cast<int>(minY / tileWorldWidth);
        tileExtent.xLength = static_cast<int>(maxX - minX / tileWorldWidth);
        tileExtent.yLength = static_cast<int>(maxY - minY / tileWorldWidth);

        return tileExtent;
    }
};

} // End namespace AM
