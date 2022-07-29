#include "PolygonTools.h"
#include <cmath>

namespace AM
{

bool PolygonTools::pointIsInsideConvexPolygon(const SDL_Point* enclosingPolygon,
                                              std::size_t vertexCount,
                                              const SDL_Point& testPoint)
{
    const int sideCount{static_cast<int>(vertexCount)};
    int sameSideResultCount{0};

    // Iterate over each side.
    for (size_t i = 0; i < sideCount; ++i) {
        const double pointInLine{calcPointRelationToLine(
            enclosingPolygon[i], enclosingPolygon[(i + 1) % sideCount],
            testPoint)};

        // Check if the point lies on the polygon.
        if (pointInLine == 0) {
            return true;
        }

        sameSideResultCount += (pointInLine > 0);
    }

    return (std::abs(sameSideResultCount) == sideCount) ? true : false;
}

double PolygonTools::calcPointRelationToLine(const SDL_Point& lineStart,
                                          const SDL_Point& lineEnd,
                                          const SDL_Point& testPoint)
{
    return ((testPoint.y - lineStart.y) * (lineEnd.x - lineStart.x))
           - ((testPoint.x - lineStart.x) * (lineEnd.y - lineStart.y));
}

} // End namespace AM
