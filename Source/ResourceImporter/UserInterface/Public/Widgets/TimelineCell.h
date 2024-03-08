#pragma once

#include "SpriteID.h"
#include "AUI/Widget.h"
#include <functional>

namespace AM
{
namespace ResourceImporter
{
/**
 * An individual cell in the animation timeline.
 */
class TimelineCell : public AUI::Widget
{
public:
    TimelineCell();

    /** If true, the "this cell has a sprite" circle should be drawn. */
    bool hasSprite;

    /** If true, this cell will draw a darker background.
        Used for every 5th cell. */
    bool drawDarkBackground;

    //-------------------------------------------------------------------------
    // Callback registration
    //-------------------------------------------------------------------------
    /**
     * @param inOnPressed A callback for when this cell is pressed.
     */
    void setOnPressed(std::function<void(void)> inOnPressed);

    //-------------------------------------------------------------------------
    // Base class overrides
    //-------------------------------------------------------------------------
    void render(const SDL_Point& windowTopLeft) override;

    AUI::EventResult onMouseDown(AUI::MouseButtonType buttonType,
                            const SDL_Point& cursorPosition) override;

    AUI::EventResult onMouseDoubleClick(AUI::MouseButtonType buttonType,
                                   const SDL_Point& cursorPosition) override;

private:
    std::function<void(void)> onPressed;
};

} // End namespace ResourceImporter
} // End namespace AM
