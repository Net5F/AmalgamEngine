#include "SpriteTools.h"
#include "Position.h"
#include "Transforms.h"
#include "Camera.h"
#include "Log.h"

namespace AM
{
namespace ResourceImporter
{
BoundingBox 
SpriteTools::calcSpriteStageWorldExtent(const SDL_Rect& spriteImageExtent,
    const SDL_Point& stageOrigin)
{
    // There are 3 options for how we could've sized the stage:
    //   1. Clamp max values to stay within the image bounds (would form an 
    //      irregular shape in-world).
    //   2. Form a world-space axis-aligned box starting at the origin, make it
    //      as large as possible while never leaving the image bounds.
    //   3. Same as #2, but make it oversized so the whole image (below the 
    //      origin) is within the stage.
    // We chose to go with #3, because it lets us use the whole image, and 
    // is easier to implement than #1. The downside is that it lets you draw 
    // boxes that go outside of the image, but that's easy to avoid if you 
    // don't want to.

    // Find the screen-space position of the images bottom corners, relative 
    // to the origin.
    SDL_FPoint bottomLeft{
        static_cast<float>(-1.f * stageOrigin.x),
        static_cast<float>(spriteImageExtent.h - stageOrigin.y)};
    SDL_FPoint bottomRight{
        static_cast<float>(spriteImageExtent.w - stageOrigin.x),
        static_cast<float>(spriteImageExtent.h - stageOrigin.y)};

    // Convert the screen-space points to world space.
    Position worldBottomLeft{Transforms::screenToWorldMinimum(bottomLeft, {})};
    Position worldBottomRight{
        Transforms::screenToWorldMinimum(bottomRight, {})};

    // Calculate the stage's extent from the boundary points.
    BoundingBox stageWorldExtent{{0, 0, 0}, {0, 0, 0}};
    stageWorldExtent.max.x = worldBottomRight.x;
    stageWorldExtent.max.y = worldBottomLeft.y;
    // Z must span from the lowest point on the screen (maxX, maxY, 0), to 
    // the top of the image.
    float originToMaxY{Transforms::worldToScreen(
        {worldBottomRight.x, worldBottomLeft.y, 0}, 1.f).y};
    stageWorldExtent.max.z = Transforms::screenYToWorldZ(
        (static_cast<float>(stageOrigin.y) + originToMaxY), 1.0);

    return stageWorldExtent;
}

} // End namespace ResourceImporter
} // End namespace AM
