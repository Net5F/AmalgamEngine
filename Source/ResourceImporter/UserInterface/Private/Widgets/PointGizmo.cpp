#include "PointGizmo.h"
#include "Transforms.h"
#include "Camera.h"
#include "Position.h"
#include "SpriteTools.h"
#include "SDLHelpers.h"
#include "SharedConfig.h"
#include "Log.h"
#include "AUI/Core.h"
#include "AUI/ScalingHelpers.h"
#include <algorithm>
#include <SDL3/SDL_rect.h>

namespace AM
{
namespace ResourceImporter
{
PointGizmo::PointGizmo(const SDL_FRect& inLogicalExtent)
: AUI::Widget(inLogicalExtent, "PointGizmo")
, lastUsedScreenSize{0, 0}
, point{}
, isEnabled{true}
, scaledRectSize{AUI::ScalingHelpers::logicalToActual(LOGICAL_RECT_SIZE)}
, logicalSpriteImageExtent{0, 0, 0, 0}
, logicalStageOrigin{0, 0}
, stageWorldExtent{}
, pointControlExtent{0, 0, scaledRectSize, scaledRectSize}
, currentlyHeld{false}
{
}

void PointGizmo::enable()
{
    isEnabled = true;
    refreshGraphics();
}

void PointGizmo::disable()
{
    isEnabled = false;
    refreshGraphics();
}

void PointGizmo::setSpriteImageSize(float logicalSpriteWidth,
                                    float logicalSpriteHeight)
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

void PointGizmo::setStageOrigin(const SDL_FPoint& inLogicalStageOrigin)
{
    logicalStageOrigin = inLogicalStageOrigin;
    updateStageExtent();
    refreshGraphics();
}

void PointGizmo::setPoint(const Vector3& newPoint)
{
    point = newPoint;
    refreshGraphics();
}

const SDL_FRect& PointGizmo::getLogicalCenteredSpriteExtent() const
{
    return logicalSpriteImageExtent;
}

void PointGizmo::setOnPointUpdated(
    std::function<void(const Vector3&)> inOnPointUpdated)
{
    onPointUpdated = std::move(inOnPointUpdated);
}

void PointGizmo::setLogicalExtent(const SDL_FRect& inLogicalExtent)
{
    Widget::setLogicalExtent(inLogicalExtent);
    updateStageExtent();
    refreshGraphics();
}

void PointGizmo::arrange(const SDL_FPoint& startPosition,
                         const SDL_FRect& availableExtent,
                         AUI::WidgetLocator* widgetLocator)
{
    // Note: This custom arrange isn't really needed, since ResourceImporter
    //       isn't likely to change screen size at runtime. It's nice to keep
    //       as an example though, of what to do if you have custom graphics
    //       that rely on clippedExtent for layout.

    // Run the normal arrange step.
    Widget::arrange(startPosition, availableExtent, widgetLocator);

    // If this widget is fully clipped, return early.
    if (SDL_RectEmptyFloat(&clippedExtent)) {
        return;
    }

    // If the UI scaling has changed, refresh everything.
    // Note: This has to be done after arranging, since it uses clippedExtent.
    if (lastUsedScreenSize != AUI::Core::getActualScreenSize()) {
        refreshScaling();
        lastUsedScreenSize = AUI::Core::getActualScreenSize();
    }
}

void PointGizmo::render(const SDL_FPoint& windowTopLeft)
{
    // If this widget is fully clipped, don't render it.
    if (SDL_RectEmptyFloat(&clippedExtent)) {
        return;
    }

    // If this gizmo is disabled, make it semi-transparent.
    float alpha{BASE_ALPHA};
    if (!isEnabled) {
        alpha *= DISABLED_ALPHA_FACTOR;
    }

    // Render the point.
    SDL_FRect offsetExtent{pointControlExtent};
    offsetExtent.x += windowTopLeft.x;
    offsetExtent.y += windowTopLeft.y;

    SDL_SetRenderDrawBlendMode(AUI::Core::getRenderer(), SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(AUI::Core::getRenderer(), 0, 0, 0,
                           static_cast<Uint8>(alpha));
    SDL_RenderFillRect(AUI::Core::getRenderer(), &offsetExtent);
}

AUI::EventResult PointGizmo::onMouseDown(AUI::MouseButtonType buttonType,
                                         const SDL_FPoint& cursorPosition)
{
    // Do nothing if disabled. Only respond to the left mouse button.
    if (!isEnabled || (buttonType != AUI::MouseButtonType::Left)) {
        return AUI::EventResult{.wasHandled{false}};
    }

    // Check if the mouse press hit our control.
    if (SDL_PointInRectFloat(&cursorPosition, &pointControlExtent)) {
        currentlyHeld = true;
    }

    // If the cursor is holding a control, set mouse capture so we get the
    // associated MouseUp.
    if (currentlyHeld) {
        return AUI::EventResult{.wasHandled{true}, .setMouseCapture{this}};
    }
    else {
        return AUI::EventResult{.wasHandled{true}};
    }
}

AUI::EventResult PointGizmo::onMouseUp(AUI::MouseButtonType buttonType,
                                       const SDL_FPoint&)
{
    // Only respond to the left mouse button.
    if (buttonType != AUI::MouseButtonType::Left) {
        return AUI::EventResult{.wasHandled{false}};
    }

    // If we're holding a control, release it and release mouse capture.
    if (currentlyHeld) {
        currentlyHeld = false;
        return AUI::EventResult{.wasHandled{true}, .releaseMouseCapture{true}};
    }
    else {
        return AUI::EventResult{.wasHandled{true}};
    }
}

AUI::EventResult PointGizmo::onMouseMove(const SDL_FPoint& cursorPosition)
{
    // If a control isn't currently being held, ignore the event.
    if (!currentlyHeld) {
        return AUI::EventResult{.wasHandled{false}};
    }

    /* Translate the mouse position to world space. */
    // Account for this widget's position.
    SDL_FRect actualSpriteImageExtent{
        AUI::ScalingHelpers::logicalToActual(logicalSpriteImageExtent)};
    SDL_FPoint actualStageOrigin{
        AUI::ScalingHelpers::logicalToActual(logicalStageOrigin)};
    float finalXOffset{clippedExtent.x + actualSpriteImageExtent.x
                       + actualStageOrigin.x};
    float finalYOffset{clippedExtent.y + actualSpriteImageExtent.y
                       + actualStageOrigin.y};

    // Apply the offset to the mouse position and convert to logical space.
    SDL_FPoint offsetMousePoint{cursorPosition.x - finalXOffset,
                                cursorPosition.y - finalYOffset};
    offsetMousePoint = AUI::ScalingHelpers::actualToLogical(offsetMousePoint);

    // Convert the screen-space mouse point to world space.
    Position mouseWorldPos{
        Transforms::screenToWorldMinimum(offsetMousePoint, {})};

    // Adjust the currently pressed control appropriately.
    updatePoint(mouseWorldPos);

    return AUI::EventResult{.wasHandled{true}};
}

void PointGizmo::refreshScaling()
{
    // Re-calculate our control rectangle size.
    scaledRectSize = AUI::ScalingHelpers::logicalToActual(LOGICAL_RECT_SIZE);

    // Refresh our controls to reflect the new sizes.
    refreshGraphics();
}

void PointGizmo::updateStageExtent()
{
    stageWorldExtent = SpriteTools::calcSpriteStageWorldExtent(
        logicalSpriteImageExtent, logicalStageOrigin);
}

void PointGizmo::refreshGraphics()
{
    // Calculate where the point is on the screen.
    SDL_FPoint screenPoint{Transforms::worldToScreen(point, 1.f)};

    // Account for this widget's position and the image's position.
    SDL_FRect actualSpriteImageExtent{
        AUI::ScalingHelpers::logicalToActual(logicalSpriteImageExtent)};
    SDL_FPoint actualStageOrigin{
        AUI::ScalingHelpers::logicalToActual(logicalStageOrigin)};
    float finalXOffset{clippedExtent.x + actualSpriteImageExtent.x
                       + actualStageOrigin.x};
    float finalYOffset{clippedExtent.y + actualSpriteImageExtent.y
                       + actualStageOrigin.y};

    // Scale and round the point.
    screenPoint.x
        = std::round(AUI::ScalingHelpers::logicalToActual(screenPoint.x));
    screenPoint.y
        = std::round(AUI::ScalingHelpers::logicalToActual(screenPoint.y));

    // Offset the point.
    screenPoint.x += finalXOffset;
    screenPoint.y += finalYOffset;

    // Calc half the control rectangle size so we can center the control.
    float halfRectSize{scaledRectSize / 2.f};

    // Move the control to the correct position.
    pointControlExtent.x = screenPoint.x - halfRectSize;
    pointControlExtent.y = screenPoint.y - halfRectSize;
}

void PointGizmo::updatePoint(const Position& mouseWorldPos)
{
    // TODO: If shift is held, only move along the Z axis

    // Note: The expected behavior is to move along the x/y plane and
    //       leave minZ where it was.
    Vector3 updatedPoint{point};
    float& x{updatedPoint.x};
    float& y{updatedPoint.y};

    // Move the point to its new position.
    x = mouseWorldPos.x;
    y = mouseWorldPos.y;

    // If we moved below the model-space origin (0, 0), bring the point back
    // in.
    if (x < 0) {
        x = 0;
    }
    if (y < 0) {
        y = 0;
    }

    // If we moved outside the stage bounds, bring the point back in.
    if (x > stageWorldExtent.max.x) {
        x = stageWorldExtent.max.x;
    }
    if (y > stageWorldExtent.max.y) {
        y = stageWorldExtent.max.y;
    }

    // Signal the updated point.
    // Note: We don't update our internal point until our owner saves the
    //       update in the model and it signals us.
    if (onPointUpdated) {
        onPointUpdated(updatedPoint);
    }
}

} // End namespace ResourceImporter
} // End namespace AM
