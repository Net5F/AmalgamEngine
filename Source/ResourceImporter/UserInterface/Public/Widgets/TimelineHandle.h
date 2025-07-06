#pragma once

#include "AUI/Widget.h"
#include <functional>

namespace AUI
{
class WidgetLocator;
}

namespace AM
{
namespace ResourceImporter
{

/**
 * A handle that can be dragged around to manipulate the timeline.
 */
class TimelineHandle : public AUI::Widget
{
public:
    TimelineHandle(const SDL_Rect& inLogicalExtent);

    /**
     * Sets the color of this widget's graphics.
     */
    void setColor(const SDL_Color& inColor);

    /**
     * Sets whether the line part of the handle graphic should be rendered.
     */
    void setRenderLine(bool inRenderLine);

    //-------------------------------------------------------------------------
    // Callback registration
    //-------------------------------------------------------------------------
    /**
     * @param inOnDragged A callback that expects the cursor's current position.
     */
    void setOnDragged(std::function<void(const SDL_Point&)> inOnDragged);

    //-------------------------------------------------------------------------
    // Base class overrides
    //-------------------------------------------------------------------------
    void arrange(const SDL_Point& startPosition,
                 const SDL_Rect& availableExtent,
                 AUI::WidgetLocator* widgetLocator) override;

    void render(const SDL_Point& windowTopLeft) override;

    AUI::EventResult onMouseDown(AUI::MouseButtonType buttonType,
                            const SDL_Point& cursorPosition) override;

    AUI::EventResult onMouseDoubleClick(AUI::MouseButtonType buttonType,
                                   const SDL_Point& cursorPosition) override;

    AUI::EventResult onMouseUp(AUI::MouseButtonType buttonType,
                               const SDL_Point& cursorPosition) override;

    AUI::EventResult onMouseMove(const SDL_Point& cursorPosition) override;

private:
    std::function<void(const SDL_Point&)> onDragged;

    /** The color of this widget's graphics. */
    SDL_Color color;

    /** If true, the line part of the handle graphic should be rendered. */
    bool renderLine;

    /** If true, the mouse is currently dragging this scrubber. */
    bool isDragging;

    /** The extent of the scrubber's rect that covers the frame numbers. */
    SDL_Rect rectLogicalExtent;

    /** The rect's actual clipped extent. */
    SDL_Rect rectClippedExtent;

    /** The extent of the scrubber's line that covers the cell. */
    SDL_Rect lineLogicalExtent;

    /** The line's actual clipped extent. */
    SDL_Rect lineClippedExtent;
};

} // End namespace ResourceImporter
} // End namespace AM
