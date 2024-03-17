#pragma once

#include "SpriteID.h"
#include "AUI/Widget.h"
#include <functional>

namespace AM
{
namespace ResourceImporter
{
/**
 * An individual frame in the animation timeline.
 *
 * Frames can be selected by clicking on them. For frames that contain sprites, 
 * you can right click and drag to move the sprite to a different frame.
 */
class TimelineFrame : public AUI::Widget
{
public:
    /** This widget's width. */
    static constexpr int LOGICAL_WIDTH{24};

    TimelineFrame();

    /** If true, this frame contains a sprite. */
    bool hasSprite;

    /** If true, this frame will draw a darker background.
        Used for every 5th frame. */
    bool drawDarkBackground;

    //-------------------------------------------------------------------------
    // Callback registration
    //-------------------------------------------------------------------------
    /**
     * @param inOnPressed A callback for when this frame is pressed.
     */
    void setOnPressed(std::function<void(void)> inOnPressed);

    /**
     * Called when this frame is right clicked while it has a sprite.
     * @param inOnSpriteDragStarted A callback that expects the cursor's current 
     *                              position.
     */
    void setOnSpriteDragStarted(
        std::function<void(const SDL_Point&)> inOnSpriteDragStarted);

    /**
     * @param inOnSpriteDragged A callback that expects the cursor's current 
     *                          position.
     */
    void setOnSpriteDragged(
        std::function<void(const SDL_Point&)> inOnSpriteDragged);

    /**
     * @param inOnSpriteDragReleased A callback that expects the cursor's 
     *                               current position.
     */
    void setOnSpriteDragReleased(
        std::function<void(const SDL_Point&)> inOnSpriteDragReleased);

    //-------------------------------------------------------------------------
    // Base class overrides
    //-------------------------------------------------------------------------
    void render(const SDL_Point& windowTopLeft) override;

    AUI::EventResult onMouseDown(AUI::MouseButtonType buttonType,
                            const SDL_Point& cursorPosition) override;

    AUI::EventResult onMouseDoubleClick(AUI::MouseButtonType buttonType,
                                   const SDL_Point& cursorPosition) override;

    AUI::EventResult onMouseUp(AUI::MouseButtonType buttonType,
                               const SDL_Point& cursorPosition) override;

    AUI::EventResult onMouseMove(const SDL_Point& cursorPosition) override;

private:
    std::function<void(void)> onPressed;
    std::function<void(const SDL_Point&)> onSpriteDragStarted;
    std::function<void(const SDL_Point&)> onSpriteDragged;
    std::function<void(const SDL_Point&)> onSpriteDragReleased;

    /** If true, the mouse is currently dragging this frame. */
    bool isDragging;
};

} // End namespace ResourceImporter
} // End namespace AM
