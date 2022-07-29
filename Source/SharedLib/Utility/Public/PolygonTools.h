#pragma once

#include <SDL_rect.h>

/**
 * This file contains helper functions for working with polygons.
 *
 * Reference: https://github.com/anirudhtopiwala/OpenSource_Problems/tree/master/Point_In_Polygon
 */
namespace AM
{
class PolygonTools
{
public:
    /**
     * Tests if the given point is inside the given polygon.
     *
     * @param enclosingPolygon  An array of points that define a polygon.
     *                          The points must be arranged in the array as a 
     *                          clockwise path around the polygon.
     * @param vertexCount  The number of elements in enclosingPolygon.
     * @param testPoint  The point to test.
     *
     * @return true if the point is inside or on the edge of the polygon, 
     *         else false.
     */
    static bool pointIsInsideConvexPolygon(const SDL_Point* enclosingPolygon,
                                           std::size_t vertexCount,
                                           const SDL_Point& testPoint);

private:
    /**
     * Tests if the test point lies on the left or right side of the given 
     * line when viewed in the anticlockwise direction.
     *
     * @return: > 0: Test point lies to the left of the line.
     *          = 0: Test point lies on the line.
     *          < 0: Test point lies to the right of the line.
     */
    static double calcPointRelationToLine(const SDL_Point& lineStart,
                                          const SDL_Point& lineEnd,
                                          const SDL_Point& testPoint);
};

} // End namespace AM
