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
    static constexpr float LOGICAL_WIDTH{24};

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
        std::function<void(const SDL_FPoint&)> inOnSpriteDragStarted);

    /**
     * @param inOnSpriteDragged A callback that expects the cursor's current
     *                          position.
     */
    void setOnSpriteDragged(
        std::function<void(const SDL_FPoint&)> inOnSpriteDragged);

    /**
     * @param inOnSpriteDragReleased A callback that expects the cursor's
     *                               current position.
     */
    void setOnSpriteDragReleased(
        std::function<void(const SDL_FPoint&)> inOnSpriteDragReleased);

    //-------------------------------------------------------------------------
    // Base class overrides
    //-------------------------------------------------------------------------
    void render(const SDL_FPoint& windowTopLeft) override;

    AUI::EventResult onMouseDown(AUI::MouseButtonType buttonType,
                                 const SDL_FPoint& cursorPosition) override;

    AUI::EventResult
        onMouseDoubleClick(AUI::MouseButtonType buttonType,
                           const SDL_FPoint& cursorPosition) override;

    AUI::EventResult onMouseUp(AUI::MouseButtonType buttonType,
                               const SDL_FPoint& cursorPosition) override;

    AUI::EventResult onMouseMove(const SDL_FPoint& cursorPosition) override;

private:
    std::function<void(void)> onPressed;
    std::function<void(const SDL_FPoint&)> onSpriteDragStarted;
    std::function<void(const SDL_FPoint&)> onSpriteDragged;
    std::function<void(const SDL_FPoint&)> onSpriteDragReleased;

    /** If true, the mouse is currently dragging this frame. */
    bool isDragging;
};

} // End namespace ResourceImporter
} // End namespace AM
