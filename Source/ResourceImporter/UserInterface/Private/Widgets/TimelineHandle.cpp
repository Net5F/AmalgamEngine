#include "TimelineHandle.h"
#include "AUI/Core.h"
#include "AUI/WidgetLocator.h"
#include "AUI/ScalingHelpers.h"

namespace AM
{
namespace ResourceImporter
{
TimelineHandle::TimelineHandle(const SDL_FRect& inLogicalExtent)
: AUI::Widget(inLogicalExtent, "TimelineHandle")
, color{24, 155, 243, 191}
, renderLine{true}
, isDragging{false}
, rectLogicalExtent{0, 0, 24, 24}
, rectClippedExtent{0, 0, 0, 0}
, lineLogicalExtent{9, 24, 6, 24}
, lineClippedExtent{0, 0, 0, 0}
{
}

void TimelineHandle::setOnDragged(
    std::function<void(const SDL_FPoint&)> inOnDragged)
{
    onDragged = std::move(inOnDragged);
}

void TimelineHandle::setColor(const SDL_Color& inColor)
{
    color = inColor;
}

void TimelineHandle::setRenderLine(bool inRenderLine)
{
    renderLine = inRenderLine;
}

void TimelineHandle::arrange(const SDL_FPoint& startPosition,
                               const SDL_FRect& availableExtent,
                               AUI::WidgetLocator* widgetLocator)
{
    // Run the normal arrange step.
    Widget::arrange(startPosition, availableExtent, widgetLocator);

    // If this widget is fully clipped, return early.
    if (SDL_RectEmptyFloat(&clippedExtent)) {
        return;
    }

    // Note: One could imagine creating widgets to wrap these graphics so 
    //       they could be automatically managed by the layout system.
    //       It seems very heavyweight to add them to the widget tree just 
    //       for layout/rendering though, so we handle this manually.
    SDL_FRect rectOffsetExtent{rectLogicalExtent};
    rectOffsetExtent.x += logicalExtent.x;
    rectOffsetExtent.y += logicalExtent.y;
    rectClippedExtent = AUI::ScalingHelpers::logicalToClipped(
        rectOffsetExtent, startPosition, availableExtent);

    SDL_FRect lineOffsetExtent{lineLogicalExtent};
    lineOffsetExtent.x += logicalExtent.x;
    lineOffsetExtent.y += logicalExtent.y;
    lineClippedExtent = AUI::ScalingHelpers::logicalToClipped(
        lineOffsetExtent, startPosition, availableExtent);
}

void TimelineHandle::render(const SDL_FPoint& windowTopLeft)
{
    // Render the rect.
    SDL_SetRenderDrawBlendMode(AUI::Core::getRenderer(), SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(AUI::Core::getRenderer(), color.r, color.g, color.b,
                           color.a);
    if (!SDL_RectEmptyFloat(&rectClippedExtent)) {
        SDL_FRect finalExtent{rectClippedExtent};
        finalExtent.x += windowTopLeft.x;
        finalExtent.y += windowTopLeft.y;
        SDL_RenderFillRect(AUI::Core::getRenderer(), &finalExtent);
    }

    // Render the line.
    if (renderLine && !SDL_RectEmptyFloat(&lineClippedExtent)) {
        SDL_FRect finalExtent{lineClippedExtent};
        finalExtent.x += windowTopLeft.x;
        finalExtent.y += windowTopLeft.y;
        SDL_RenderFillRect(AUI::Core::getRenderer(), &finalExtent);
    }
}

AUI::EventResult TimelineHandle::onMouseDown(AUI::MouseButtonType buttonType,
                                           const SDL_FPoint& cursorPosition)
{
    // Only respond to the left mouse button.
    if (buttonType != AUI::MouseButtonType::Left) {
        return AUI::EventResult{.wasHandled{false}};
    }

    isDragging = true;

    return AUI::EventResult{.wasHandled{true}, .setMouseCapture{this}};
}

AUI::EventResult
    TimelineHandle::onMouseDoubleClick(AUI::MouseButtonType buttonType,
                                     const SDL_FPoint& cursorPosition)
{
    // We treat additional clicks as regular MouseDown events.
    return onMouseDown(buttonType, cursorPosition);
}

AUI::EventResult TimelineHandle::onMouseUp(AUI::MouseButtonType buttonType,
                                             const SDL_FPoint& cursorPosition)
{
    if (isDragging) {
        isDragging = false;
        return AUI::EventResult{.wasHandled{true}};
    }

    return AUI::EventResult{.wasHandled{false}, .releaseMouseCapture{true}};
}

AUI::EventResult TimelineHandle::onMouseMove(const SDL_FPoint& cursorPosition)
{
    if (isDragging && onDragged) {
        onDragged(cursorPosition);
        return AUI::EventResult{.wasHandled{true}};
    }

    return AUI::EventResult{.wasHandled{false}};
}

} // End namespace ResourceImporter
} // End namespace AM
