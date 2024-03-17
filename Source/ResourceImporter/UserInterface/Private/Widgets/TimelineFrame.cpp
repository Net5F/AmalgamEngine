#include "TimelineFrame.h"
#include "AUI/Core.h"
#include "AUI/ScalingHelpers.h"
#include <SDL2_gfxPrimitives.h>

namespace AM
{
namespace ResourceImporter
{
TimelineFrame::TimelineFrame()
: AUI::Widget({0, 0, LOGICAL_WIDTH, 26}, "TimelineFrame")
, hasSprite{false}
, drawDarkBackground{false}
, isDragging{false}
{
}

void TimelineFrame::setOnPressed(std::function<void(void)> inOnPressed)
{
    onPressed = std::move(inOnPressed);
}

void TimelineFrame::setOnSpriteDragStarted(
    std::function<void(const SDL_Point&)> inOnSpriteDragStarted)
{
    onSpriteDragStarted = std::move(inOnSpriteDragStarted);
}

void TimelineFrame::setOnSpriteDragged(
    std::function<void(const SDL_Point&)> inOnSpriteDragged)
{
    onSpriteDragged = std::move(inOnSpriteDragged);
}

void TimelineFrame::setOnSpriteDragReleased(
    std::function<void(const SDL_Point&)> inOnSpriteDragReleased)
{
    onSpriteDragReleased = std::move(inOnSpriteDragReleased);
}

void TimelineFrame::render(const SDL_Point& windowTopLeft)
{
    // Render the larger outline rectangle.
    SDL_Rect finalExtent{clippedExtent};
    finalExtent.x += windowTopLeft.x;
    finalExtent.y += windowTopLeft.y;
    SDL_SetRenderDrawColor(AUI::Core::getRenderer(), 184, 184, 184, 255);
    SDL_RenderFillRect(AUI::Core::getRenderer(), &finalExtent);

    // Render the body rectangle.
    const int borderSize{AUI::ScalingHelpers::logicalToActual(1)};
    finalExtent.x += borderSize;
    finalExtent.y += borderSize;
    finalExtent.w -= (borderSize * 2);
    finalExtent.h -= (borderSize * 2);
    if (!drawDarkBackground) {
        SDL_SetRenderDrawColor(AUI::Core::getRenderer(), 230, 230, 230, 255);
    }
    SDL_RenderFillRect(AUI::Core::getRenderer(), &finalExtent);

    // If this frame has a sprite, render the circle.
    if (hasSprite) {
        SDL_Point frameCenter{(finalExtent.x + (finalExtent.w / 2)),
                              (finalExtent.y + (finalExtent.h / 2))};
        int radius{AUI::ScalingHelpers::logicalToActual(7)};
        filledCircleRGBA(AUI::Core::getRenderer(), frameCenter.x, frameCenter.y,
                         radius, 64, 64, 64, 255);
    }
}

AUI::EventResult TimelineFrame::onMouseDown(AUI::MouseButtonType buttonType,
                                           const SDL_Point& cursorPosition)
{
    // If the right mouse button was pressed and we have a sprite to drag, 
    // start dragging.
    if ((buttonType == AUI::MouseButtonType::Right) && hasSprite) {
        isDragging = true;

        if (onSpriteDragStarted) {
            onSpriteDragStarted(cursorPosition);
        }

        return AUI::EventResult{.wasHandled{true}, .setMouseCapture{this}};
    }
    // Otherwise, only respond to the left mouse button.
    else if (buttonType != AUI::MouseButtonType::Left) {
        return AUI::EventResult{.wasHandled{false}};
    }

    // If the user set a callback, call it.
    if (onPressed) {
        onPressed();
    }

    return AUI::EventResult{.wasHandled{true}};
}

AUI::EventResult
    TimelineFrame::onMouseDoubleClick(AUI::MouseButtonType buttonType,
                                     const SDL_Point& cursorPosition)
{
    // We treat additional clicks as regular MouseDown events.
    return onMouseDown(buttonType, cursorPosition);
}

AUI::EventResult TimelineFrame::onMouseUp(AUI::MouseButtonType buttonType,
                                         const SDL_Point& cursorPosition)
{
    if (isDragging) {
        isDragging = false;

        if (onSpriteDragReleased) {
            onSpriteDragReleased(cursorPosition);
        }

        return AUI::EventResult{.wasHandled{true}};
    }

    return AUI::EventResult{.wasHandled{false}, .releaseMouseCapture{true}};
}

AUI::EventResult TimelineFrame::onMouseMove(const SDL_Point& cursorPosition)
{
    if (isDragging) {
        if (onSpriteDragged) {
            onSpriteDragged(cursorPosition);
        }

        return AUI::EventResult{.wasHandled{true}};
    }

    return AUI::EventResult{.wasHandled{false}};
}

} // End namespace ResourceImporter
} // End namespace AM
