#include "TimelineScrubber.h"
#include "AUI/Core.h"
#include "AUI/WidgetLocator.h"
#include "AUI/ScalingHelpers.h"

namespace AM
{
namespace ResourceImporter
{
TimelineScrubber::TimelineScrubber(const SDL_Point& inLogicalOrigin)
: AUI::Widget({inLogicalOrigin.x, inLogicalOrigin.y, 18, 48},
              "TimelineScrubber")
, isDragging{false}
, rectLogicalExtent{0, 0, 18, 22}
, rectClippedExtent{0, 0, 0, 0}
, lineLogicalExtent{7, 18, 4, 26}
, lineClippedExtent{0, 0, 0, 0}
{
}

void TimelineScrubber::setOnDragged(
    std::function<void(const SDL_Point&)> inOnDragged)
{
    onDragged = std::move(inOnDragged);
}

void TimelineScrubber::updateLayout(const SDL_Point& startPosition,
                                    const SDL_Rect& availableExtent,
                                    WidgetLocator* widgetLocator)
{
    // Note: You could imagine creating widgets to wrap these graphics so 
    //       they could be automatically managed by the layout system.
    //       It seems very heavyweight to add them to the widget tree just 
    //       for layout/rendering though, so we handle this manually.
    rectClippedExtent = AUI::ScalingHelpers::logicalToClipped(
        rectLogicalExtent, startPosition, availableExtent);
    lineClippedExtent = AUI::ScalingHelpers::logicalToClipped(
        lineLogicalExtent, startPosition, availableExtent);
}

void TimelineScrubber::render(const SDL_Point& windowTopLeft)
{
    // Render the rect.
    SDL_SetRenderDrawColor(AUI::Core::getRenderer(), 24, 155, 243, 191);
    if (!SDL_RectEmpty(&rectClippedExtent)) {
        SDL_RenderFillRect(AUI::Core::getRenderer(), &rectClippedExtent);
    }

    // Render the line.
    if (!SDL_RectEmpty(&lineClippedExtent)) {
        SDL_RenderFillRect(AUI::Core::getRenderer(), &lineClippedExtent);
    }
}

AUI::EventResult TimelineScrubber::onMouseDown(AUI::MouseButtonType buttonType,
                                           const SDL_Point& cursorPosition)
{
    // Only respond to the left mouse button.
    if (buttonType != AUI::MouseButtonType::Left) {
        return AUI::EventResult{.wasHandled{false}};
    }

    isDragging = true;

    return AUI::EventResult{.wasHandled{true}};
}

AUI::EventResult
    TimelineScrubber::onMouseDoubleClick(AUI::MouseButtonType buttonType,
                                     const SDL_Point& cursorPosition)
{
    // We treat additional clicks as regular MouseDown events.
    return onMouseDown(buttonType, cursorPosition);
}

AUI::EventResult TimelineScrubber::onMouseUp(AUI::MouseButtonType buttonType,
                                             const SDL_Point& cursorPosition)
{
    if (isDragging) {
        isDragging = false;
        return AUI::EventResult{.wasHandled{true}};
    }

    return AUI::EventResult{.wasHandled{false}};
}

AUI::EventResult TimelineScrubber::onMouseMove(const SDL_Point& cursorPosition)
{
    if (isDragging && onDragged) {
        onDragged(cursorPosition);
        return AUI::EventResult{.wasHandled{true}};
    }

    return AUI::EventResult{.wasHandled{false}};
}

} // End namespace ResourceImporter
} // End namespace AM
