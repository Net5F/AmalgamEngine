#include "BoundingBoxGizmo.h"
#include "Transforms.h"
#include "Camera.h"
#include "Position.h"
#include "SpriteTools.h"
#include "SharedConfig.h"
#include "Log.h"
#include "AUI/Core.h"
#include "AUI/ScalingHelpers.h"
#include "AUI/SDLHelpers.h"
#include <algorithm>
#include <SDL_rect.h>
#include <SDL2_gfxPrimitives.h>

namespace AM
{
namespace ResourceImporter
{
BoundingBoxGizmo::BoundingBoxGizmo(const SDL_Rect& inLogicalExtent)
: AUI::Widget(inLogicalExtent, "BoundingBoxGizmo")
, lastUsedScreenSize{0, 0}
, boundingBox{}
, isEnabled{true}
, scaledRectSize{AUI::ScalingHelpers::logicalToActual(LOGICAL_RECT_SIZE)}
, scaledLineWidth{AUI::ScalingHelpers::logicalToActual(LOGICAL_LINE_WIDTH)}
, logicalSpriteImageExtent{0, 0, 0, 0}
, logicalStageOrigin{0, 0}
, stageWorldExtent{}
, positionControlExtent{0, 0, scaledRectSize, scaledRectSize}
, xControlExtent{0, 0, scaledRectSize, scaledRectSize}
, yControlExtent{0, 0, scaledRectSize, scaledRectSize}
, zControlExtent{0, 0, scaledRectSize, scaledRectSize}
, lineXMinPoint{}
, lineXMaxPoint{}
, lineYMinPoint{}
, lineYMaxPoint{}
, lineZMinPoint{}
, lineZMaxPoint{}
, planeXCoords{}
, planeYCoords{}
, currentHeldControl{Control::None}
{
}

void BoundingBoxGizmo::enable()
{
    isEnabled = true;
    refreshGraphics();
}

void BoundingBoxGizmo::disable()
{
    isEnabled = false;
    refreshGraphics();
}

void BoundingBoxGizmo::setSpriteImageSize(int logicalSpriteWidth,
                                          int logicalSpriteHeight)
{
    logicalSpriteImageExtent.x
        = (logicalExtent.w / 2) - (logicalSpriteWidth / 2);
    logicalSpriteImageExtent.y
        = (logicalExtent.h / 2) - (logicalSpriteHeight / 2);
    logicalSpriteImageExtent.w = logicalSpriteWidth;
    logicalSpriteImageExtent.h = logicalSpriteHeight;

    updateStageExtent();
    refreshGraphics();
}

void BoundingBoxGizmo::setStageOrigin(const SDL_Point& inLogicalStageOrigin)
{
    logicalStageOrigin = inLogicalStageOrigin;
    updateStageExtent();
    refreshGraphics();
}

void BoundingBoxGizmo::setBoundingBox(const BoundingBox& newBoundingBox)
{
    boundingBox = newBoundingBox;
    refreshGraphics();
}

const SDL_Rect& BoundingBoxGizmo::getLogicalCenteredSpriteExtent() const
{
    return logicalSpriteImageExtent;
}

void BoundingBoxGizmo::setOnBoundingBoxUpdated(
    std::function<void(const BoundingBox&)> inOnBoundingBoxUpdated)
{
    onBoundingBoxUpdated = std::move(inOnBoundingBoxUpdated);
}

void BoundingBoxGizmo::setLogicalExtent(const SDL_Rect& inLogicalExtent)
{
    Widget::setLogicalExtent(inLogicalExtent);
    updateStageExtent();
    refreshGraphics();
}

void BoundingBoxGizmo::arrange(const SDL_Point& startPosition,
                               const SDL_Rect& availableExtent,
                               AUI::WidgetLocator* widgetLocator)
{
    // Note: This custom arrange isn't really needed, since ResourceImporter
    //       isn't likely to change screen size at runtime. It's nice to keep
    //       as an example though, of what to do if you have custom graphics 
    //       that rely on clippedExtent for layout.

    // Run the normal arrange step.
    Widget::arrange(startPosition, availableExtent, widgetLocator);

    // If this widget is fully clipped, return early.
    if (SDL_RectEmpty(&clippedExtent)) {
        return;
    }

    // If the UI scaling has changed, refresh everything.
    // Note: This has to be done after arranging, since it uses clippedExtent.
    if (lastUsedScreenSize != AUI::Core::getActualScreenSize()) {
        refreshScaling();
        lastUsedScreenSize = AUI::Core::getActualScreenSize();
    }
}

void BoundingBoxGizmo::render(const SDL_Point& windowTopLeft)
{
    // If this widget is fully clipped, don't render it.
    if (SDL_RectEmpty(&clippedExtent)) {
        return;
    }

    renderPlanes(windowTopLeft);
    renderLines(windowTopLeft);
    renderControls(windowTopLeft);
}

AUI::EventResult BoundingBoxGizmo::onMouseDown(AUI::MouseButtonType buttonType,
                                               const SDL_Point& cursorPosition)
{
    // Do nothing if disabled. Only respond to the left mouse button.
    if (!isEnabled || (buttonType != AUI::MouseButtonType::Left)) {
        return AUI::EventResult{.wasHandled{false}};
    }

    // Check if the mouse press hit any of our controls.
    if (SDL_PointInRect(&cursorPosition, &positionControlExtent)) {
        currentHeldControl = Control::Position;
    }
    else if (SDL_PointInRect(&cursorPosition, &xControlExtent)) {
        currentHeldControl = Control::X;
    }
    else if (SDL_PointInRect(&cursorPosition, &yControlExtent)) {
        currentHeldControl = Control::Y;
    }
    else if (SDL_PointInRect(&cursorPosition, &zControlExtent)) {
        currentHeldControl = Control::Z;
    }

    // If the cursor is holding a control, set mouse capture so we get the
    // associated MouseUp.
    if (currentHeldControl != Control::None) {
        return AUI::EventResult{.wasHandled{true}, .setMouseCapture{this}};
    }
    else {
        return AUI::EventResult{.wasHandled{true}};
    }
}

AUI::EventResult BoundingBoxGizmo::onMouseUp(AUI::MouseButtonType buttonType,
                                             const SDL_Point&)
{
    // Only respond to the left mouse button.
    if (buttonType != AUI::MouseButtonType::Left) {
        return AUI::EventResult{.wasHandled{false}};
    }

    // If we're holding a control, release it and release mouse capture.
    if (currentHeldControl != Control::None) {
        currentHeldControl = Control::None;
        return AUI::EventResult{.wasHandled{true}, .releaseMouseCapture{true}};
    }
    else {
        return AUI::EventResult{.wasHandled{true}};
    }
}

AUI::EventResult BoundingBoxGizmo::onMouseMove(const SDL_Point& cursorPosition)
{
    // If a control isn't currently being held, ignore the event.
    if (currentHeldControl == Control::None) {
        return AUI::EventResult{.wasHandled{false}};
    }

    /* Translate the mouse position to world space. */
    // Account for this widget's position.
    SDL_Rect actualSpriteImageExtent{
        AUI::ScalingHelpers::logicalToActual(logicalSpriteImageExtent)};
    SDL_Point actualStageOrigin{
        AUI::ScalingHelpers::logicalToActual(logicalStageOrigin)};
    int finalXOffset{clippedExtent.x + actualSpriteImageExtent.x
                     + actualStageOrigin.x};
    int finalYOffset{clippedExtent.y + actualSpriteImageExtent.y
                     + actualStageOrigin.y};

    // Apply the offset to the mouse position and convert to logical space.
    SDL_Point offsetMousePoint{cursorPosition.x - finalXOffset,
                               cursorPosition.y - finalYOffset};
    offsetMousePoint = AUI::ScalingHelpers::actualToLogical(offsetMousePoint);

    // Convert the screen-space mouse point to world space.
    SDL_FPoint offsetMouseScreenPoint{static_cast<float>(offsetMousePoint.x),
                                      static_cast<float>(offsetMousePoint.y)};
    Position mouseWorldPos{
        Transforms::screenToWorldMinimum(offsetMouseScreenPoint, {})};

    // Adjust the currently pressed control appropriately.
    switch (currentHeldControl) {
        case Control::Position: {
            updatePositionBounds(mouseWorldPos);
            break;
        }
        case Control::X: {
            updateXBounds(mouseWorldPos);
            break;
        }
        case Control::Y: {
            updateYBounds(mouseWorldPos);
            break;
        }
        case Control::Z: {
            updateZBounds(cursorPosition.y);
            break;
        }
        default: {
            break;
        }
    }

    return AUI::EventResult{.wasHandled{true}};
}

void BoundingBoxGizmo::refreshScaling()
{
    // Re-calculate our control rectangle size.
    scaledRectSize = AUI::ScalingHelpers::logicalToActual(LOGICAL_RECT_SIZE);

    // Re-calculate our line width.
    scaledLineWidth = AUI::ScalingHelpers::logicalToActual(LOGICAL_LINE_WIDTH);

    // Refresh our controls to reflect the new sizes.
    refreshGraphics();
}

void BoundingBoxGizmo::updateStageExtent()
{
    stageWorldExtent = SpriteTools::calcSpriteStageWorldExtent(
        logicalSpriteImageExtent, logicalStageOrigin);
}

void BoundingBoxGizmo::refreshGraphics()
{
    // Calculate where the bounding box is on the screen.
    std::vector<SDL_Point> screenPoints{};
    calcBoundingBoxScreenPoints(screenPoints);

    // Move the controls, lines, and planes to the correct positions.
    moveControls(screenPoints);
    moveLines(screenPoints);
    movePlanes(screenPoints);
}

void BoundingBoxGizmo::updatePositionBounds(const Position& mouseWorldPos)
{
    // TODO: If shift is held, only move along the Z axis

    // Note: The expected behavior is to move along the x/y plane and
    //       leave minZ where it was.
    BoundingBox updatedBounds{boundingBox};
    float& minX{updatedBounds.min.x};
    float& minY{updatedBounds.min.y};
    float& maxX{updatedBounds.max.x};
    float& maxY{updatedBounds.max.y};

    // Move the min bounds to follow the max bounds.
    float diffX{mouseWorldPos.x - maxX};
    float diffY{mouseWorldPos.y - maxY};
    minX += diffX;
    minY += diffY;

    // Move the max bounds to their new position.
    maxX = mouseWorldPos.x;
    maxY = mouseWorldPos.y;

    // If we moved below the model-space origin (0, 0), bring the box bounds
    // back in.
    if (minX < 0) {
        maxX += -minX;
        minX = 0;
    }
    if (minY < 0) {
        maxY += -minY;
        minY = 0;
    }

    // If we moved outside the stage bounds, bring the box bounds back in.
    if (maxX > stageWorldExtent.max.x) {
        float diff{maxX - stageWorldExtent.max.x};
        minX -= diff;
        maxX -= diff;
    }
    if (maxY > stageWorldExtent.max.y) {
        float diff{maxY - stageWorldExtent.max.y};
        minY -= diff;
        maxY -= diff;
    }

    // Signal the updated bounding box.
    // Note: We don't update our internal bounding box until our owner saves the
    //       update in the model and it signals us.
    if (onBoundingBoxUpdated) {
        onBoundingBoxUpdated(updatedBounds);
    }
}

void BoundingBoxGizmo::updateXBounds(const Position& mouseWorldPos)
{
    // Clamp the new value to its bounds.
    BoundingBox updatedBounds{boundingBox};
    updatedBounds.min.x = std::clamp(mouseWorldPos.x, 0.f, updatedBounds.max.x);

    // Signal the updated bounding box.
    // Note: We don't update our internal bounding box until our owner 
    //       saves the update in the model and calls setBoundingBox().
    if (onBoundingBoxUpdated) {
        onBoundingBoxUpdated(updatedBounds);
    }
}

void BoundingBoxGizmo::updateYBounds(const Position& mouseWorldPos)
{
    // Clamp the new value to its bounds.
    BoundingBox updatedBounds{boundingBox};
    updatedBounds.min.y = std::clamp(mouseWorldPos.y, 0.f, updatedBounds.max.y);

    // Signal the updated bounding box.
    if (onBoundingBoxUpdated) {
        onBoundingBoxUpdated(updatedBounds);
    }
}

void BoundingBoxGizmo::updateZBounds(int mouseScreenYPos)
{
    // Note: The screenToWorld() transformation can't handle z-axis
    // movement (not enough data from a 2d point), so we have to do it
    // using our contextual information.

    // Set maxZ relative to the distance between the mouse and the
    // position control (the position control is always our reference
    // for where minimum Z is.)
    float mouseZHeight{positionControlExtent.y + (scaledRectSize / 2.f)
                       - mouseScreenYPos};

    // Convert to logical space.
    mouseZHeight = AUI::ScalingHelpers::actualToLogical(mouseZHeight);

    // Apply our screen -> world Z scaling.
    mouseZHeight = Transforms::screenYToWorldZ(mouseZHeight, 1.f);

    // Set maxZ, making sure it doesn't go below minZ or beyond the stage.
    BoundingBox updatedBounds{boundingBox};
    updatedBounds.max.z
        = std::clamp(mouseZHeight, updatedBounds.min.z, stageWorldExtent.max.z);

    // Signal the updated bounding box.
    if (onBoundingBoxUpdated) {
        onBoundingBoxUpdated(updatedBounds);
    }
}

void BoundingBoxGizmo::calcBoundingBoxScreenPoints(
    std::vector<SDL_Point>& boundsScreenPoints)
{
    /* Transform the world positions to screen points. */
    std::array<SDL_FPoint, 7> screenPoints{};

    // Push the points in the correct order.
    const Vector3& minPoint{boundingBox.min};
    const Vector3& maxPoint{boundingBox.max};
    Vector3 point{minPoint.x, maxPoint.y, minPoint.z};
    screenPoints[0] = Transforms::worldToScreen(point, 1);

    point = {maxPoint.x, maxPoint.y, minPoint.z};
    screenPoints[1] = Transforms::worldToScreen(point, 1);

    point = {maxPoint.x, minPoint.y, minPoint.z};
    screenPoints[2] = Transforms::worldToScreen(point, 1);

    point = {minPoint.x, maxPoint.y, maxPoint.z};
    screenPoints[3] = Transforms::worldToScreen(point, 1);

    point = {maxPoint.x, maxPoint.y, maxPoint.z};
    screenPoints[4] = Transforms::worldToScreen(point, 1);

    point = {maxPoint.x, minPoint.y, maxPoint.z};
    screenPoints[5] = Transforms::worldToScreen(point, 1);

    point = {minPoint.x, minPoint.y, maxPoint.z};
    screenPoints[6] = Transforms::worldToScreen(point, 1);

    // Account for this widget's position and the image's position.
    SDL_Rect actualSpriteImageExtent{
        AUI::ScalingHelpers::logicalToActual(logicalSpriteImageExtent)};
    SDL_Point actualStageOrigin{
        AUI::ScalingHelpers::logicalToActual(logicalStageOrigin)};
    int finalXOffset{clippedExtent.x + actualSpriteImageExtent.x
                     + actualStageOrigin.x};
    int finalYOffset{clippedExtent.y + actualSpriteImageExtent.y
                     + actualStageOrigin.y};

    // Scale and offset each point, then push it into the return vector.
    for (SDL_FPoint& point : screenPoints) {
        // Scale and round the point.
        point.x = std::round(AUI::ScalingHelpers::logicalToActual(point.x));
        point.y = std::round(AUI::ScalingHelpers::logicalToActual(point.y));

        // Offset the point.
        point.x += finalXOffset;
        point.y += finalYOffset;

        // Cast to int and push into the return vector.
        boundsScreenPoints.push_back(
            {static_cast<int>(point.x), static_cast<int>(point.y)});
    }
}

void BoundingBoxGizmo::moveControls(std::vector<SDL_Point>& boundsScreenPoints)
{
    // Calc half the control rectangle size so we can center the controls.
    int halfRectSize{static_cast<int>(scaledRectSize / 2.f)};

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

void BoundingBoxGizmo::moveLines(std::vector<SDL_Point>& boundsScreenPoints)
{
    // Move the lines.
    lineXMinPoint = {boundsScreenPoints[0].x, boundsScreenPoints[0].y};
    lineXMaxPoint = {boundsScreenPoints[1].x, boundsScreenPoints[1].y};

    lineYMinPoint = {boundsScreenPoints[2].x, boundsScreenPoints[2].y};
    lineYMaxPoint = {boundsScreenPoints[1].x, boundsScreenPoints[1].y};

    lineZMinPoint = {boundsScreenPoints[1].x, boundsScreenPoints[1].y};
    lineZMaxPoint = {boundsScreenPoints[4].x, boundsScreenPoints[4].y};
}

void BoundingBoxGizmo::movePlanes(std::vector<SDL_Point>& boundsScreenPoints)
{
    // Set the coords for the X-axis plane (coords 0 - 3, starting from top
    // left and going clockwise.)
    planeXCoords[0] = boundsScreenPoints[4].x;
    planeYCoords[0] = boundsScreenPoints[4].y;
    planeXCoords[1] = boundsScreenPoints[5].x;
    planeYCoords[1] = boundsScreenPoints[5].y;
    planeXCoords[2] = boundsScreenPoints[2].x;
    planeYCoords[2] = boundsScreenPoints[2].y;
    planeXCoords[3] = boundsScreenPoints[1].x;
    planeYCoords[3] = boundsScreenPoints[1].y;

    // Set the coords for the Y-axis plane (coords 4 - 7, starting from top
    // left and going clockwise.)
    planeXCoords[4] = boundsScreenPoints[3].x;
    planeYCoords[4] = boundsScreenPoints[3].y;
    planeXCoords[5] = boundsScreenPoints[4].x;
    planeYCoords[5] = boundsScreenPoints[4].y;
    planeXCoords[6] = boundsScreenPoints[1].x;
    planeYCoords[6] = boundsScreenPoints[1].y;
    planeXCoords[7] = boundsScreenPoints[0].x;
    planeYCoords[7] = boundsScreenPoints[0].y;

    // Set the coords for the Z-axis plane (coords 8 - 11, starting from top
    // left and going clockwise.)
    planeXCoords[8] = boundsScreenPoints[6].x;
    planeYCoords[8] = boundsScreenPoints[6].y;
    planeXCoords[9] = boundsScreenPoints[5].x;
    planeYCoords[9] = boundsScreenPoints[5].y;
    planeXCoords[10] = boundsScreenPoints[4].x;
    planeYCoords[10] = boundsScreenPoints[4].y;
    planeXCoords[11] = boundsScreenPoints[3].x;
    planeYCoords[11] = boundsScreenPoints[3].y;
}

void BoundingBoxGizmo::renderControls(const SDL_Point& windowTopLeft)
{
    // If this gizmo is disabled, make it semi-transparent.
    float alpha{BASE_ALPHA};
    if (!isEnabled) {
        alpha *= DISABLED_ALPHA_FACTOR;
    }

    // Position control
    SDL_Rect offsetExtent{positionControlExtent};
    offsetExtent.x += windowTopLeft.x;
    offsetExtent.y += windowTopLeft.y;

    SDL_SetRenderDrawBlendMode(AUI::Core::getRenderer(), SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(AUI::Core::getRenderer(), 0, 0, 0,
                           static_cast<Uint8>(alpha));
    SDL_RenderFillRect(AUI::Core::getRenderer(), &offsetExtent);

    // X control
    offsetExtent = xControlExtent;
    offsetExtent.x += windowTopLeft.x;
    offsetExtent.y += windowTopLeft.y;

    SDL_SetRenderDrawColor(AUI::Core::getRenderer(), 148, 0, 0,
                           static_cast<Uint8>(alpha));
    SDL_RenderFillRect(AUI::Core::getRenderer(), &offsetExtent);

    // Y control
    offsetExtent = yControlExtent;
    offsetExtent.x += windowTopLeft.x;
    offsetExtent.y += windowTopLeft.y;

    SDL_SetRenderDrawColor(AUI::Core::getRenderer(), 0, 149, 0,
                           static_cast<Uint8>(alpha));
    SDL_RenderFillRect(AUI::Core::getRenderer(), &offsetExtent);

    // Z control
    offsetExtent = zControlExtent;
    offsetExtent.x += windowTopLeft.x;
    offsetExtent.y += windowTopLeft.y;

    SDL_SetRenderDrawColor(AUI::Core::getRenderer(), 0, 82, 240,
                           static_cast<Uint8>(alpha));
    SDL_RenderFillRect(AUI::Core::getRenderer(), &offsetExtent);
}

void BoundingBoxGizmo::renderLines(const SDL_Point& windowTopLeft)
{
    // If this gizmo is disabled, make it semi-transparent.
    float alpha{BASE_ALPHA};
    if (!isEnabled) {
        alpha *= DISABLED_ALPHA_FACTOR;
    }

    // X-axis line
    SDL_Point offsetMinPoint{lineXMinPoint};
    SDL_Point offsetMaxPoint{lineXMaxPoint};
    offsetMinPoint.x += windowTopLeft.x;
    offsetMinPoint.y += windowTopLeft.y;
    offsetMaxPoint.x += windowTopLeft.x;
    offsetMaxPoint.y += windowTopLeft.y;

    thickLineRGBA(AUI::Core::getRenderer(), offsetMinPoint.x, offsetMinPoint.y,
                  offsetMaxPoint.x, offsetMaxPoint.y, scaledLineWidth, 148, 0,
                  0, static_cast<Uint8>(alpha));

    // Y-axis line
    offsetMinPoint = lineYMinPoint;
    offsetMaxPoint = lineYMaxPoint;
    offsetMinPoint.x += windowTopLeft.x;
    offsetMinPoint.y += windowTopLeft.y;
    offsetMaxPoint.x += windowTopLeft.x;
    offsetMaxPoint.y += windowTopLeft.y;

    thickLineRGBA(AUI::Core::getRenderer(), offsetMinPoint.x, offsetMinPoint.y,
                  offsetMaxPoint.x, offsetMaxPoint.y, scaledLineWidth, 0, 149,
                  0, static_cast<Uint8>(alpha));

    // Z-axis line
    offsetMinPoint = lineZMinPoint;
    offsetMaxPoint = lineZMaxPoint;
    offsetMinPoint.x += windowTopLeft.x;
    offsetMinPoint.y += windowTopLeft.y;
    offsetMaxPoint.x += windowTopLeft.x;
    offsetMaxPoint.y += windowTopLeft.y;

    thickLineRGBA(AUI::Core::getRenderer(), offsetMinPoint.x, offsetMinPoint.y,
                  offsetMaxPoint.x, offsetMaxPoint.y, scaledLineWidth, 0, 82,
                  240, static_cast<Uint8>(alpha));
}

void BoundingBoxGizmo::renderPlanes(const SDL_Point& windowTopLeft)
{
    /* Offset all the points. */
    std::array<Sint16, 12> offsetXCoords{};
    for (std::size_t i = 0; i < offsetXCoords.size(); ++i) {
        offsetXCoords[i] = planeXCoords[i] + windowTopLeft.x;
    }

    std::array<Sint16, 12> offsetYCoords{};
    for (std::size_t i = 0; i < offsetYCoords.size(); ++i) {
        offsetYCoords[i] = planeYCoords[i] + windowTopLeft.y;
    }

    /* Draw the planes. */
    // If this gizmo is disabled, make it semi-transparent.
    float alpha{BASE_ALPHA * PLANE_ALPHA_FACTOR};
    if (!isEnabled) {
        alpha *= DISABLED_ALPHA_FACTOR;
    }

    // X-axis plane
    filledPolygonRGBA(AUI::Core::getRenderer(), &(offsetXCoords[0]),
                      &(offsetYCoords[0]), 4, 0, 149, 0,
                      static_cast<Uint8>(alpha));

    // Y-axis plane
    filledPolygonRGBA(AUI::Core::getRenderer(), &(offsetXCoords[4]),
                      &(offsetYCoords[4]), 4, 148, 0, 0,
                      static_cast<Uint8>(alpha));

    // Z-axis plane
    filledPolygonRGBA(AUI::Core::getRenderer(), &(offsetXCoords[8]),
                      &(offsetYCoords[8]), 4, 0, 82, 240,
                      static_cast<Uint8>(alpha));
}

} // End namespace ResourceImporter
} // End namespace AM
