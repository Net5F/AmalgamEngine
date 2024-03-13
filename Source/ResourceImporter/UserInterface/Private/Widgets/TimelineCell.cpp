#include "TimelineCell.h"
#include "AUI/Core.h"
#include "AUI/ScalingHelpers.h"
#include <SDL2_gfxPrimitives.h>

namespace AM
{
namespace ResourceImporter
{
TimelineCell::TimelineCell()
: AUI::Widget({0, 0, LOGICAL_WIDTH, 26}, "TimelineCell")
, hasSprite{false}
, drawDarkBackground{false}
{
}

void TimelineCell::setOnPressed(std::function<void(void)> inOnPressed)
{
    onPressed = std::move(inOnPressed);
}

void TimelineCell::render(const SDL_Point& windowTopLeft)
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

    // If this cell has a sprite, render the circle.
    if (hasSprite) {
        SDL_Point cellCenter{(finalExtent.x + (finalExtent.w / 2)),
                             (finalExtent.y + (finalExtent.h / 2))};
        int radius{AUI::ScalingHelpers::logicalToActual(7)};
        filledCircleRGBA(AUI::Core::getRenderer(), cellCenter.x, cellCenter.y,
                         radius, 64, 64, 64, 255);
    }
}

AUI::EventResult TimelineCell::onMouseDown(AUI::MouseButtonType buttonType,
                                           const SDL_Point& cursorPosition)
{
    // Only respond to the left mouse button.
    if (buttonType != AUI::MouseButtonType::Left) {
        return AUI::EventResult{.wasHandled{false}};
    }

    // If the user set a callback, call it.
    if (onPressed) {
        onPressed();
    }

    return AUI::EventResult{.wasHandled{true}};
}

AUI::EventResult
    TimelineCell::onMouseDoubleClick(AUI::MouseButtonType buttonType,
                                     const SDL_Point& cursorPosition)
{
    // We treat additional clicks as regular MouseDown events.
    return onMouseDown(buttonType, cursorPosition);
}

} // End namespace ResourceImporter
} // End namespace AM
