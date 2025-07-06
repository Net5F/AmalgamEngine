#pragma once

#include "TimelineHandle.h"
#include "Timer.h"
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

    void setLoopStartFrame(Uint8 newLoopStartFrame);

    void setFrame(Uint8 frameNumber, bool hasSprite);

    /**
     * If an animation isn't currently playing, plays the active animation 
     * from the start. If an animation is currently playing, pauses it.
     */
    void playOrPauseAnimation();

    /**
     * @return The currently selected frame (the one that the scrubber is 
     *         hovering over).
     */
    Uint8 getSelectedFrameNumber() const;

    //-------------------------------------------------------------------------
    // Callback registration
    //-------------------------------------------------------------------------
    /**
     * @param inOnSelectionChanged A callback that expects the new selected 
     *                             frame's number.
     */
    void setOnSelectionChanged(
        std::function<void(Uint8 selectedFrameNumber)> inOnSelectionChanged);

    /**
     * @param inOnLoopStartFrameChanged A callback that expects the new 
     *                                  loop start frame.
     */
    void setOnLoopStartFrameChanged(
        std::function<void(Uint8 newLoopStartFrame)> inOnLoopStartFrameChanged);

    /**
     * @param inOnSpriteMoved A callback that expects the old and new frame 
     *                        indices.
     */
    void setOnSpriteMoved(
        std::function<void(Uint8 oldFrameNumber, Uint8 newFrameNumber)>
            inOnSpriteMoved);

    //-------------------------------------------------------------------------
    // Base class overrides
    //-------------------------------------------------------------------------
    void onTick(double timestepS) override;

    void render(const SDL_Point& windowTopLeft) override;

private:
    /**
     * If the scrubber has been dragged far enough, selects a new frame.
     */
    void onFrameScrubberDragged(const SDL_Point& cursorPosition);

    // TODO: We're trying this to see if it's performant enough. If updating 
    //       while dragging is too slow, we can do it like SpriteDrag
    /**
     * If the handle has been dragged far enough, changes the loop value.
     */
    void onLoopHandleDragged(const SDL_Point& cursorPosition);

    /**
     * Handles a frame that contains a sprite being right-click dragged.
     */
    void onSpriteDragStarted(Uint8 frameNumber,
                             const SDL_Point& cursorPosition);

    /**
     * If a frame containing a sprite has been dragged far enough, visually 
     * moves the frame circle to the new hovered frame.
     */
    void onSpriteDragged(Uint8 frameNumber, const SDL_Point& cursorPosition);

    /**
     * If the cursor is over a frame other than originSpriteDragFrameNumber, 
     * moves the sprite into that frame.
     */
    void onSpriteDragReleased(Uint8 frameNumber,
                              const SDL_Point& cursorPosition);

    /**
     * Refreshes frameContainer to match the current active animation.
     */
    void refreshFrames();

    /**
     * Moves the frame scrubber to the given frame.
     */
    void setFrameScrubberPosition(Uint8 frameNumber);

    /**
     * Moves the loop handle to the given frame.
     */
    void setLoopHandlePosition(Uint8 frameNumber);

    /** 
     * Returns which frame the given cursor position is aligned with.
     */
    Uint8 getCursorFrame(const SDL_Point& cursorPosition);

    void styleNumberText(AUI::Text& textObject, const std::string& text);

    /** Holds the number text that goes above the frames. */
    AUI::HorizontalGridContainer numberContainer;

    /** Holds the frames that contain the animation's frames. */
    AUI::HorizontalGridContainer frameContainer;

    /** Used to select how many frames will be looped.
        Note: This needs to be behind frameScrubber since it's double-tall to 
              account for the handle (even though we don't render it). */
    TimelineHandle loopHandle;

    /** Used to move between frames. */
    TimelineHandle frameScrubber;

    /** The current selected frame. */
    Uint8 selectedFrameNumber;

    /** The animation that is currently loaded into this timeline. */
    const EditorAnimation* activeAnimation;

    /** If a sprite is being dragged, this holds the original frame number. */
    Uint8 originSpriteDragFrameNumber;

    /** If a sprite is being dragged, this holds the current frame number that 
        the sprite is being dragged over. */
    Uint8 currentSpriteDragFrameNumber;

    /** Used for properly pacing the animation during playback. */
    Timer animationTimer;

    /** Accumulates time to keep animation playback smooth. */
    double animationAccumulator;

    /** If true, we're currently playing the animation. */
    bool playingAnimation;

    /** The amber background that we render to show the loop area. */
    SDL_Rect loopBackgroundExtent;

    std::function<void(Uint8)> onSelectionChanged;

    std::function<void(Uint8)> onLoopStartFrameChanged;

    std::function<void(Uint8, Uint8)> onSpriteMoved;
};

} // End namespace ResourceImporter
} // End namespace AM
