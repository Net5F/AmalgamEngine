#include "StageGraphic.h"
#include "SpriteTools.h"
#include "Transforms.h"
#include "BoundingBox.h"
#include "SDLHelpers.h"
#include "AUI/Core.h"
#include "AUI/ScalingHelpers.h"

namespace AM
{
namespace ResourceImporter
{
StageGraphic::StageGraphic(const SDL_FRect& inLogicalExtent)
: AUI::Widget({320, 58, 1297, 1022}, "StageGraphic")
, stageCoords{}
{
}

void StageGraphic::updateStage(const SDL_FRect& spriteTextureExtent,
                               const SDL_FPoint& stageOrigin,
                               const SDL_FPoint& actualSpriteImageOffset)
{
    // We need to update where the stage is on the screen for our generated 
    // graphics. To do that, we must transform the stage's world positions to 
    // screen points.
    std::array<SDL_FPoint, 4> screenPoints{};

    // Push the points in the correct order.
    BoundingBox stageWorldExtent{SpriteTools::calcSpriteStageWorldExtent(
        spriteTextureExtent, stageOrigin)};
    const Vector3& minPoint{stageWorldExtent.min};
    const Vector3& maxPoint{stageWorldExtent.max};
    Vector3 point{minPoint.x, minPoint.y, minPoint.z};
    screenPoints[0] = Transforms::worldToScreen(point, 1);

    point = {maxPoint.x, minPoint.y, minPoint.z};
    screenPoints[1] = Transforms::worldToScreen(point, 1);

    point = {maxPoint.x, maxPoint.y, minPoint.z};
    screenPoints[2] = Transforms::worldToScreen(point, 1);

    point = {minPoint.x, maxPoint.y, minPoint.z};
    screenPoints[3] = Transforms::worldToScreen(point, 1);

    // Account for the gizmo's position and the image's position.
    SDL_FPoint actualStageOrigin{
        AUI::ScalingHelpers::logicalToActual(stageOrigin)};
    float finalXOffset{actualSpriteImageOffset.x + actualStageOrigin.x};
    float finalYOffset{actualSpriteImageOffset.y + actualStageOrigin.y};

    // Scale and offset each point, then update stageCoords.
    for (std::size_t i{0}; i < screenPoints.size(); ++i) {
        SDL_FPoint& screenPoint{screenPoints[i]};

        // Scale and round the point.
        screenPoint.x
            = std::round(AUI::ScalingHelpers::logicalToActual(screenPoint.x));
        screenPoint.y
            = std::round(AUI::ScalingHelpers::logicalToActual(screenPoint.y));

        // Offset the point.
        screenPoint.x += finalXOffset;
        screenPoint.y += finalYOffset;

        // Update the graphic's coordinate.
        stageCoords[i].x = screenPoint.x;
        stageCoords[i].y = screenPoint.y;
    }
}

void StageGraphic::render(const SDL_FPoint& windowTopLeft)
{
    // If this widget is fully clipped, don't render it.
    if (SDL_RectEmptyFloat(&clippedExtent)) {
        return;
    }

    renderStage(windowTopLeft);
}

void StageGraphic::renderStage(const SDL_FPoint& windowTopLeft)
{
    // Offset all the points.
    std::array<SDL_Vertex, 4> verts{};
    for (std::size_t i{0}; i < verts.size(); ++i) {
        verts[i].position.x = stageCoords[i].x + windowTopLeft.x;
        verts[i].position.y = stageCoords[i].y + windowTopLeft.y;
        verts[i].color = SDLHelpers::colorToFColor(
            {0, 149, 0, static_cast<Uint8>(STAGE_ALPHA)});
    }

    // Draw the stage's floor bounds.
    // Two triangles form a face: (0, 1, 2) and (0, 2, 3)
    std::array<int, 6> indices{0, 1, 2, 0, 2, 3};
    SDL_RenderGeometry(AUI::Core::getRenderer(), nullptr, verts.data(), 4,
                       indices.data(), 6);
}

} // End namespace ResourceImporter
} // End namespace AM
