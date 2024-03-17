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

    /**
     * @return The currently selected frame (the one that the scrubber is 
     *         hovering over).
     */
    Uint8 getSelectedFrameNumber();

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

    /**
     * @param inOnSpriteMoved A callback that expects the old frame, new frame, 
     *                        and the sprite that is being moved.
     */
    void setOnSpriteMoved(
        std::function<void(Uint8 oldFrameIndex, Uint8 newFrameIndex,
                           const EditorSprite*)>
            inOnSpriteMoved);

private:
    /**
     * If the scrubber has been dragged far enough, selects a new frame.
     */
    void onScrubberDragged(const SDL_Point& cursorPosition);

    /**
     * Handles a frame that contains a sprite being right-click dragged.
     */
    void onSpriteDragStarted(Uint8 frameIndex, const SDL_Point& cursorPosition);

    /**
     * If a frame containing a sprite has been dragged far enough, visually 
     * moves the frame circle to the new hovered frame.
     */
    void onSpriteDragged(Uint8 frameIndex, const SDL_Point& cursorPosition);

    /**
     * If the cursor is over a frame other than originDragFrameIndex, moves the 
     * sprite into that frame.
     */
    void onSpriteDragReleased(Uint8 frameIndex, const SDL_Point& cursorPosition);

    /**
     * Refreshes frameContainer to match the current active animation.
     */
    void refreshFrames();

    /**
     * Moves the scrubber to the given frame.
     */
    void setSelectedFrame(Uint8 frameNumber);

    /** 
     * Returns which frame the given cursor position is aligned with.
     */
    Uint8 getCursorFrame(const SDL_Point& cursorPosition);

    /**
     * Returns the sprite in the given frame of the current active animation, 
     * or nullptr if the frame doesn't have a sprite.
     */
    const EditorSprite* getSpriteFromFrame(Uint8 frameNumber);

    void styleNumberText(AUI::Text& textObject, const std::string& text);

    /** Holds the number text that goes above the frames. */
    AUI::HorizontalGridContainer numberContainer;

    /** Holds the frames that contain the animation's frames. */
    AUI::HorizontalGridContainer frameContainer;

    /** Used to move between frames. */
    TimelineScrubber scrubber;

    /** The current selected frame. */
    Uint8 selectedFrameNumber;

    /** The animation that is currently loaded into this timeline. */
    const EditorAnimation* activeAnimation;

    /** If a sprite is being dragged, this holds the original frame index. */
    Uint8 originSpriteDragFrameIndex;

    /** If a sprite is being dragged, this holds the current index that the 
        sprite is being dragged over. */
    Uint8 currentSpriteDragFrameIndex;

    std::function<void(const EditorSprite*)> onSelectionChanged;

    std::function<void(Uint8, Uint8, const EditorSprite*)> onSpriteMoved;
};

} // End namespace ResourceImporter
} // End namespace AM
