#pragma once

#include "TileExtent.h"
#include "Position.h"
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
        centerPosition.x = ((maxX - minX) / 2);
        centerPosition.y = ((maxY - minY) / 2);
        centerPosition.z = ((maxZ - minZ) / 2);

        return centerPosition;
    }

    /**
     * Returns true if this box intersects the given cylinder.
     *
     * Note: This treats the given data as a cylinder with infinite length
     *       along the Z axis. If we want it to treat it as a sphere, we can
     *       change it.
     */
    bool intersects(const Position& cylinderCenter, unsigned int radius)
    {
        // TODO: If this works, comment it.
        Position boxCenter{getCenterPosition()};
        float xLength{getXLength()};
        float yLength{getYLength()};

        float circleDistanceX{std::abs(cylinderCenter.x - boxCenter.x)};
        float circleDistanceY{std::abs(cylinderCenter.y - boxCenter.y)};

        if (circleDistanceX > ((xLength / 2) + radius)) {
            return false;
        }
        if (circleDistanceY > ((yLength / 2) + radius)) {
            return false;
        }

        if (circleDistanceX <= (xLength / 2)) {
            return true;
        }
        if (circleDistanceY <= (yLength / 2)) {
            return true;
        }

        float xDif{circleDistanceX - (xLength / 2)};
        float yDif{circleDistanceY - (yLength / 2)};
        float cornerDistanceSquared{(xDif * xDif) + (yDif * yDif)};

        return (cornerDistanceSquared <= (radius * radius));
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
