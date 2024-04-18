#include "TimelineScrubber.h"
#include "AUI/Core.h"
#include "AUI/WidgetLocator.h"
#include "AUI/ScalingHelpers.h"

namespace AM
{
namespace ResourceImporter
{
TimelineScrubber::TimelineScrubber()
: AUI::Widget({0, 0, 18, 48}, "TimelineScrubber")
, isDragging{false}
, rectLogicalExtent{0, 0, 24, 24}
, rectClippedExtent{0, 0, 0, 0}
, lineLogicalExtent{9, 24, 6, 24}
, lineClippedExtent{0, 0, 0, 0}
{
}

void TimelineScrubber::setOnDragged(
    std::function<void(const SDL_Point&)> inOnDragged)
{
    onDragged = std::move(inOnDragged);
}

void TimelineScrubber::arrange(const SDL_Point& startPosition,
                               const SDL_Rect& availableExtent,
                               AUI::WidgetLocator* widgetLocator)
{
    // Run the normal arrange step.
    Widget::arrange(startPosition, availableExtent, widgetLocator);

    // If this widget is fully clipped, return early.
    if (SDL_RectEmpty(&clippedExtent)) {
        return;
    }

    // Note: One could imagine creating widgets to wrap these graphics so 
    //       they could be automatically managed by the layout system.
    //       It seems very heavyweight to add them to the widget tree just 
    //       for layout/rendering though, so we handle this manually.
    SDL_Rect rectOffsetExtent{rectLogicalExtent};
    rectOffsetExtent.x += logicalExtent.x;
    rectOffsetExtent.y += logicalExtent.y;
    rectClippedExtent = AUI::ScalingHelpers::logicalToClipped(
        rectOffsetExtent, startPosition, availableExtent);

    SDL_Rect lineOffsetExtent{lineLogicalExtent};
    lineOffsetExtent.x += logicalExtent.x;
    lineOffsetExtent.y += logicalExtent.y;
    lineClippedExtent = AUI::ScalingHelpers::logicalToClipped(
        lineOffsetExtent, startPosition, availableExtent);
}

void TimelineScrubber::render(const SDL_Point& windowTopLeft)
{
    // Render the rect.
    SDL_SetRenderDrawBlendMode(AUI::Core::getRenderer(), SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(AUI::Core::getRenderer(), 24, 155, 243, 191);
    if (!SDL_RectEmpty(&rectClippedExtent)) {
        SDL_Rect finalExtent{rectClippedExtent};
        finalExtent.x += windowTopLeft.x;
        finalExtent.y += windowTopLeft.y;
        SDL_RenderFillRect(AUI::Core::getRenderer(), &finalExtent);
    }

    // Render the line.
    if (!SDL_RectEmpty(&lineClippedExtent)) {
        SDL_Rect finalExtent{lineClippedExtent};
        finalExtent.x += windowTopLeft.x;
        finalExtent.y += windowTopLeft.y;
        SDL_RenderFillRect(AUI::Core::getRenderer(), &finalExtent);
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

    return AUI::EventResult{.wasHandled{true}, .setMouseCapture{this}};
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

    return AUI::EventResult{.wasHandled{false}, .releaseMouseCapture{true}};
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
