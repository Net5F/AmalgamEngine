#include "BoundingBoxGizmo.h"
#include "MainScreen.h"
#include "SpriteDataModel.h"
#include "TransformationHelpers.h"
#include "AUI/Core.h"
#include "AUI/ScalingHelpers.h"

namespace AM
{
namespace SpriteEditor
{

BoundingBoxGizmo::BoundingBoxGizmo(MainScreen& inScreen)
: AUI::Component(inScreen, "", {0, 0, 1920, 1080})
, mainScreen{inScreen}
, activeSprite{nullptr}
, activeSpriteUiExtent{}
, scaledRectSize{AUI::ScalingHelpers::logicalToActual(LOGICAL_RECT_SIZE)}
, positionControlExtent{0, 0, scaledRectSize, scaledRectSize}
, xControlExtent{0, 0, scaledRectSize, scaledRectSize}
, yControlExtent{0, 0, scaledRectSize, scaledRectSize}
, zControlExtent{0, 0, scaledRectSize, scaledRectSize}
{
}

void BoundingBoxGizmo::loadActiveSprite(SpriteStaticData* inActiveSprite
                                        , SDL_Rect inActiveSpriteUiExtent)
{
    // Set the new active sprite.
    activeSprite = inActiveSprite;
    activeSpriteUiExtent = inActiveSpriteUiExtent;

    // Refresh the UI with the newly set sprite's data.
    refresh();
}

void BoundingBoxGizmo::refresh()
{
    if (activeSprite == nullptr) {
        LOG_ERROR("Tried to refresh with nullptr data.");
    }

    // Calculate where the sprite's model bounds are on the screen.
    std::vector<SDL_Point> boundsScreenPoints;
    calcOffsetScreenPoints(boundsScreenPoints);

    // Move the controls to the correct positions.
    moveControls(boundsScreenPoints);

    // Move the lines to the correct positions.

    // Move the planes to the correct positions.
}

void BoundingBoxGizmo::render(const SDL_Point& parentOffset)
{
    // Keep our extent up to date.
    refreshScaling();

    // Save the extent that we're going to render at.
    lastRenderedExtent = scaledExtent;
    lastRenderedExtent.x += parentOffset.x;
    lastRenderedExtent.y += parentOffset.y;

    // If the component isn't visible, return without rendering.
    if (!isVisible) {
        return;
    }

    // Children should render at the parent's offset + this component's offset.
    SDL_Point childOffset{parentOffset};
    childOffset.x += scaledExtent.x;
    childOffset.y += scaledExtent.y;

    // Render the planes.

    // Render the lines.

    // Render the control rectangles.
    renderControls(childOffset);
}

bool BoundingBoxGizmo::refreshScaling()
{
    // If scaledExtent was refreshed, do our specialized refreshing.
    if (Component::refreshScaling()) {
        // Re-calculate our control rectangle size.
        scaledRectSize = AUI::ScalingHelpers::logicalToActual(LOGICAL_RECT_SIZE);

        return true;
    }

    return false;
}

void BoundingBoxGizmo::calcOffsetScreenPoints(std::vector<SDL_Point>& boundsScreenPoints)
{
    /* Transform the world positions to screen points. */
    // Set up a vector of float points so we can maintain precision until
    // the end.
    std::vector<ScreenPoint> floatPoints;

    // Push the points in the correct order.
    BoundingBox& modelBounds = activeSprite->modelBounds;
    Position position{modelBounds.minX, modelBounds.maxY, modelBounds.minZ};
    floatPoints.push_back(TransformationHelpers::worldToScreen(position, 1));

    position = {modelBounds.maxX, modelBounds.maxY, modelBounds.minZ};
    floatPoints.push_back(TransformationHelpers::worldToScreen(position, 1));

    position = {modelBounds.maxX, modelBounds.minY, modelBounds.minZ};
    floatPoints.push_back(TransformationHelpers::worldToScreen(position, 1));

    position = {modelBounds.minX, modelBounds.maxY, modelBounds.maxZ};
    floatPoints.push_back(TransformationHelpers::worldToScreen(position, 1));

    position = {modelBounds.maxX, modelBounds.maxY, modelBounds.maxZ};
    floatPoints.push_back(TransformationHelpers::worldToScreen(position, 1));

    position = {modelBounds.maxX, modelBounds.minY, modelBounds.maxZ};
    floatPoints.push_back(TransformationHelpers::worldToScreen(position, 1));

    position = {modelBounds.minX, modelBounds.minY, modelBounds.maxZ};
    floatPoints.push_back(TransformationHelpers::worldToScreen(position, 1));

    /* Build the offsets. */
    // Account for the sprite's position in the UI.
    int xOffset = activeSpriteUiExtent.x;
    int yOffset = activeSpriteUiExtent.y;

    // Account for the sprite's empty vertical space.
    yOffset += AUI::ScalingHelpers::logicalToActual(384);

    // Account for the sprite's half-tile offset.
    xOffset += AUI::ScalingHelpers::logicalToActual(256 / 2);

    /* Scale and offset each point, then push it into the return vector. */
    for (ScreenPoint& point : floatPoints)
    {
        // Scale and round the point.
        point.x = std::round(AUI::ScalingHelpers::logicalToActual(point.x));
        point.y = std::round(AUI::ScalingHelpers::logicalToActual(point.y));

        // Offset the point.
        point.x += xOffset;
        point.y += yOffset;

        // Cast to int and push into the return vector.
        boundsScreenPoints.push_back({static_cast<int>(point.x)
            , static_cast<int>(point.y)});
    }
}

void BoundingBoxGizmo::moveControls(std::vector<SDL_Point>& boundsScreenPoints)
{
    // Calc half the control rectangle size so we can center the controls.
    int halfRectSize = static_cast<int>(scaledRectSize / 2.f);

    // Move the control extents.
    positionControlExtent.x = boundsScreenPoints[1].x - halfRectSize;
    positionControlExtent.y = boundsScreenPoints[1].y - halfRectSize;

    xControlExtent.x = boundsScreenPoints[0].x - halfRectSize;
    xControlExtent.y = boundsScreenPoints[0].y - halfRectSize;

    yControlExtent.x = boundsScreenPoints[2].x - halfRectSize;
    yControlExtent.y = boundsScreenPoints[2].y - halfRectSize;

    zControlExtent.x = boundsScreenPoints[4].x - halfRectSize;
    zControlExtent.y = boundsScreenPoints[4].y - halfRectSize;
}

void BoundingBoxGizmo::renderControls(const SDL_Point& childOffset)
{
    // Position control
    SDL_Rect offsetExtent{positionControlExtent};
    offsetExtent.x += childOffset.x;
    offsetExtent.y += childOffset.y;
    SDL_SetRenderDrawColor(AUI::Core::GetRenderer(), 255, 0, 0, 255);
    SDL_RenderFillRect(AUI::Core::GetRenderer(), &offsetExtent);

    // X control
    offsetExtent = xControlExtent;
    offsetExtent.x += childOffset.x;
    offsetExtent.y += childOffset.y;
    SDL_SetRenderDrawColor(AUI::Core::GetRenderer(), 148, 0, 0, 255);
    SDL_RenderFillRect(AUI::Core::GetRenderer(), &offsetExtent);

    // Y control
    offsetExtent = yControlExtent;
    offsetExtent.x += childOffset.x;
    offsetExtent.y += childOffset.y;
    SDL_SetRenderDrawColor(AUI::Core::GetRenderer(), 0, 149, 0, 255);
    SDL_RenderFillRect(AUI::Core::GetRenderer(), &offsetExtent);

    // Z control
    offsetExtent = zControlExtent;
    offsetExtent.x += childOffset.x;
    offsetExtent.y += childOffset.y;
    SDL_SetRenderDrawColor(AUI::Core::GetRenderer(), 0, 82, 240, 255);
    SDL_RenderFillRect(AUI::Core::GetRenderer(), &offsetExtent);
}

} // End namespace SpriteEditor
} // End namespace AM
