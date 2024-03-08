#pragma once

#include "TimelineScrubber.h"
#include "AUI/Widget.h"
#include "AUI/Text.h"
#include "AUI/HorizontalGridContainer.h"
#include <string>
#include <functional>

namespace AM
{
namespace ResourceImporter
{
struct EditorAnimation;
struct EditorSprite;

/**
 * The animation timeline used by AnimationEditStage.
 * 
 * Lets users add sprites to an animation.
 */
class AnimationTimeline : public AUI::Widget
{
public:
    AnimationTimeline(const SDL_Rect& inLogicalExtent,
                      const std::string& inDebugName = "AnimationTimeline");

    void setActiveAnimation(const EditorAnimation& newActiveAnimation);

    void setFrameCount(Uint8 newFrameCount);

    void setFrame(Uint8 frameNumber, bool hasSprite);

    //-------------------------------------------------------------------------
    // Callback registration
    //-------------------------------------------------------------------------
    /**
     * @param inOnSelectionChanged A callback that expects the new selection's 
     *                             sprite. If nullptr, the selection has no 
     *                             sprite.
     */
    void setOnSelectionChanged(
        std::function<void(const EditorSprite*)> inOnSelectionChanged);

private:
    /**
     * Refreshes cellContainer to match the current active animation.
     */
    void refreshCells();

    /**
     * Moves the scrubber to the given frame cell.
     */
    void moveScrubberToCell(Uint8 cellIndex);

    void styleNumberText(AUI::Text& textObject, const std::string& text);

    /** Holds the number text that goes above the cells. */
    AUI::HorizontalGridContainer numberContainer;

    /** Holds the cells that contain the animation's frames. */
    AUI::HorizontalGridContainer cellContainer;

    /** Used to move between frames. */
    TimelineScrubber scrubber;

    /** The current selected frame. */
    Uint8 selectedFrameNumber;

    /** The animation that is currently loaded into this timeline. */
    const EditorAnimation* activeAnimation;

    std::function<void(const EditorSprite*)> onSelectionChanged;
};

} // End namespace ResourceImporter
} // End namespace AM
