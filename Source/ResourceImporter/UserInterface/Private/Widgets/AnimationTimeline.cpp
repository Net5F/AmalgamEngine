#include "AnimationTimeline.h"
#include "EditorAnimation.h"
#include "TimelineFrame.h"
#include "Paths.h"
#include "AMAssert.h"
#include "AUI/ScalingHelpers.h"
#include "AUI/Core.h"
#include "Log.h"
#include <algorithm>

namespace AM
{
namespace ResourceImporter
{
AnimationTimeline::AnimationTimeline(const SDL_Rect& inLogicalExtent,
                                     const std::string& inDebugName)
: AUI::Widget(inLogicalExtent, inDebugName)
, numberContainer{{0, 24 + 4, logicalExtent.w, 18}, "NumberContainer"}
, frameContainer{{0, 48, logicalExtent.w, 26}, "FrameContainer"}
, loopHandle{{0, 0, 18, 48}}
, frameScrubber{{0, 24, 18, 48}}
, selectedFrameNumber{0}
, activeAnimation{nullptr}
, originSpriteDragFrameNumber{0}
, currentSpriteDragFrameNumber{0}
, animationAccumulator{0}
, playingAnimation{false}
, loopBackgroundExtent{}
{
    // Add our children so they're included in rendering, etc.
    children.push_back(numberContainer);
    children.push_back(frameContainer);
    children.push_back(loopHandle);
    children.push_back(frameScrubber);

    /* Numbers. */
    numberContainer.setNumRows(1);
    numberContainer.setCellWidth(24 + 96);
    numberContainer.setCellHeight(18);
    numberContainer.setScrollingEnabled(false);

    /* Frame container. */
    frameContainer.setNumRows(1);
    frameContainer.setCellWidth(TimelineFrame::LOGICAL_WIDTH);
    frameContainer.setCellHeight(26);
    frameContainer.setScrollingEnabled(false);

    /* Frame handles. */
    frameScrubber.setColor({24, 155, 243, 191});
    frameScrubber.setRenderLine(true);
    loopHandle.setColor({255, 191, 0, 191});
    loopHandle.setRenderLine(false);

    frameScrubber.setOnDragged([&](const SDL_Point& cursorPosition) {
        onFrameScrubberDragged(cursorPosition);
    });
    loopHandle.setOnDragged([&](const SDL_Point& cursorPosition) {
        onLoopHandleDragged(cursorPosition);
    });
}

void AnimationTimeline::setActiveAnimation(
    const EditorAnimation& newActiveAnimation)
{
    activeAnimation = &newActiveAnimation;

    refreshFrames();

    setFrameScrubberPosition(0);
    setLoopStartFrame(activeAnimation->loopStartFrame);
}

void AnimationTimeline::setFrameCount(Uint8 newFrameCount)
{
    refreshFrames();

    // Move the scrubber if necessary.
    if (selectedFrameNumber >= newFrameCount) {
        setFrameScrubberPosition(newFrameCount - 1);
    }

    // Note: If loopStartFrame is larger than newFrameCount, the model handles 
    //       updating and signalling it before the frame count signal is 
    //       emitted.
}

void AnimationTimeline::setLoopStartFrame(Uint8 newLoopStartFrame)
{
    setLoopHandlePosition(newLoopStartFrame);
}

void AnimationTimeline::setFrame(Uint8 frameNumber, bool hasSprite)
{
    TimelineFrame& frame{
        static_cast<TimelineFrame&>(*frameContainer[frameNumber])};
    frame.hasSprite = hasSprite;

    // If the frame is currently selected, refresh it so the user gets notified 
    // of the sprite change.
    if (frameNumber == selectedFrameNumber) {
        setFrameScrubberPosition(selectedFrameNumber);
    }
}

void AnimationTimeline::playOrPauseAnimation()
{
    if (!playingAnimation) {
        setFrameScrubberPosition(0);
        animationTimer.reset();
        animationAccumulator = 0;
        playingAnimation = true;
    }
    else {
        playingAnimation = false;
    }
}

Uint8 AnimationTimeline::getSelectedFrameNumber() const
{
    return selectedFrameNumber;
}

void AnimationTimeline::setOnSelectionChanged(
    std::function<void(Uint8)> inOnSelectionChanged)
{
    onSelectionChanged = std::move(inOnSelectionChanged);
}

void AnimationTimeline::setOnLoopStartFrameChanged(
    std::function<void(Uint8 newLoopStartFrame)> inOnLoopStartFrameChanged)
{
    onLoopStartFrameChanged = std::move(inOnLoopStartFrameChanged);
}

void AnimationTimeline::setOnSpriteMoved(
    std::function<void(Uint8, Uint8)> inOnSpriteMoved)
{
    onSpriteMoved = std::move(inOnSpriteMoved);
}

void AnimationTimeline::onTick(double)
{
    // If we aren't playing an animation, do nothing.
    if (!playingAnimation) {
        return;
    }

    // If enough time has passed, go to the next frame.
    animationAccumulator += animationTimer.getTimeAndReset();
    double animationTimestepS{1
                              / static_cast<double>(activeAnimation->fps)};
    while (animationAccumulator > animationTimestepS) {
        // If we're already at the last frame, loop or end the animation.
        Uint8 frameCount{activeAnimation->frameCount};
        Uint8 loopStartFrame{activeAnimation->loopStartFrame};
        if (selectedFrameNumber == (frameCount - 1)) {
            if (loopStartFrame != frameCount) {
                // Loop back to the start frame.
                setFrameScrubberPosition(loopStartFrame);
            }
            else {
                // No loop, stop playing the animation.
                playingAnimation = false;
                return;
            }
        }
        else {
            // Proceed to the next frame.
            setFrameScrubberPosition(selectedFrameNumber + 1);
        }

        animationAccumulator -= animationTimestepS;
    }
}

void AnimationTimeline::render(const SDL_Point& windowTopLeft)
{
    // If this widget is fully clipped, don't render it.
    if (SDL_RectEmpty(&clippedExtent)) {
        return;
    }

    // Render the loop background.
    int loopCount{activeAnimation->frameCount
                  - activeAnimation->loopStartFrame};
    if (loopCount != 0) {
        int scaledFrameWidth{
            AUI::ScalingHelpers::logicalToActual(TimelineFrame::LOGICAL_WIDTH)};
        int loopBackgroundX{
            clippedExtent.x + windowTopLeft.x
            + (scaledFrameWidth * activeAnimation->loopStartFrame)};
        int loopBackgroundWidth{scaledFrameWidth * loopCount};

        SDL_Rect loopBackgroundExtent{
            loopBackgroundX, (clippedExtent.y + windowTopLeft.y),
            loopBackgroundWidth, AUI::ScalingHelpers::logicalToActual(24 * 2)};
        SDL_SetRenderDrawBlendMode(AUI::Core::getRenderer(),
                                   SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(AUI::Core::getRenderer(), 255, 191, 0, 64);
        SDL_RenderFillRect(AUI::Core::getRenderer(), &loopBackgroundExtent);
    }

    // Render our child widgets.
    Widget::render(windowTopLeft);
}

void AnimationTimeline::onFrameScrubberDragged(const SDL_Point& cursorPosition)
{
    // Ignore scrubber interactions if we're playing an animation.
    if (playingAnimation) {
        return;
    }

    Uint8 cursorFrame{getCursorFrame(cursorPosition)};

    // If the cursor isn't aligned with the current selected frame, select the 
    // new one.
    if ((cursorFrame != selectedFrameNumber)
        && (cursorFrame < frameContainer.size())) {
        setFrameScrubberPosition(cursorFrame);
    }
}

void AnimationTimeline::onLoopHandleDragged(const SDL_Point& cursorPosition)
{
    // Ignore loop handle interactions if we're playing an animation.
    if (playingAnimation) {
        return;
    }

    Uint8 cursorFrame{getCursorFrame(cursorPosition)};

    // If the cursor isn't aligned with the current loop frame, select the 
    // new one.
    // Note: The loop handle can go 1 frame past the timeline, to set 
    //       loopFrameCount to 0.
    if ((cursorFrame != activeAnimation->loopStartFrame)
        && (cursorFrame <= frameContainer.size())) {
        setLoopHandlePosition(cursorFrame);

        // If the user set a callback, call it.
        // Note: We do this here instead of in setLoopHandlePosition because 
        //       we only want to signal the change on drag (i.e. not when 
        //       setLoopStartFrame() is called).
        if (onLoopStartFrameChanged) {
            onLoopStartFrameChanged(cursorFrame);
        }
    }
}

void AnimationTimeline::onSpriteDragStarted(Uint8 frameNumber,
                                            const SDL_Point& cursorPosition)
{
    // Remember where the drag started.
    originSpriteDragFrameNumber = frameNumber;
    currentSpriteDragFrameNumber = frameNumber;
}

void AnimationTimeline::onSpriteDragged(Uint8 frameNumber,
                                        const SDL_Point& cursorPosition)
{
    // If the cursor is over a new frame, reset the old frame's circle 
    // and add one to the new frame.
    Uint8 cursorFrame{getCursorFrame(cursorPosition)};
    if (cursorFrame != currentSpriteDragFrameNumber) {
        TimelineFrame& oldCurrentFrame{static_cast<TimelineFrame&>(
            *frameContainer[currentSpriteDragFrameNumber])};
        TimelineFrame& newCurrentFrame{static_cast<TimelineFrame&>(
            *frameContainer[cursorFrame])};

        // If we're moving from the origin frame, remove its marker.
        if (currentSpriteDragFrameNumber == originSpriteDragFrameNumber) {
            oldCurrentFrame.hasSprite = false;
        }
        else {
            // Not the origin frame, return it to whatever it was.
            // Note: We don't use getSpriteAtFrame() because it may return a 
            //       sprite from a previous frame.
            auto expandedFrameVector{activeAnimation->getExpandedFrameVector()};
            oldCurrentFrame.hasSprite
                = (expandedFrameVector.at(currentSpriteDragFrameNumber)
                   != nullptr);
        }
        newCurrentFrame.hasSprite = true;

        currentSpriteDragFrameNumber = cursorFrame;
    }
}

void AnimationTimeline::onSpriteDragReleased(Uint8 frameNumber,
                                             const SDL_Point& cursorPosition)
{
    // Reset the display state.
    TimelineFrame& originFrame{static_cast<TimelineFrame&>(
        *frameContainer[originSpriteDragFrameNumber])};
    TimelineFrame& currentFrame{static_cast<TimelineFrame&>(
        *frameContainer[currentSpriteDragFrameNumber])};

    bool originHasSprite{originFrame.hasSprite};
    bool currentHasSprite{currentFrame.hasSprite};
    originFrame.hasSprite = currentHasSprite;
    currentFrame.hasSprite = originHasSprite;

    // If the cursor is over a new frame, move the sprite to it.
    if ((currentSpriteDragFrameNumber != originSpriteDragFrameNumber)
        && onSpriteMoved) {
        onSpriteMoved(originSpriteDragFrameNumber,
                      currentSpriteDragFrameNumber);
    }
}

void AnimationTimeline::refreshFrames()
{
    // Fill the frame container with empty frames to match the animation's frame 
    // count.
    frameContainer.clear();
    numberContainer.clear();
    for (Uint8 i{0}; i < activeAnimation->frameCount; ++i) {
        auto frame{std::make_unique<TimelineFrame>()};
        frame->setOnPressed([&, i]() { setFrameScrubberPosition(i); });
        frame->setOnSpriteDragStarted([&, i](const SDL_Point& cursorPosition) {
            onSpriteDragStarted(i, cursorPosition);
        });
        frame->setOnSpriteDragged([&, i](const SDL_Point& cursorPosition) {
            onSpriteDragged(i, cursorPosition);
        });
        frame->setOnSpriteDragReleased([&, i](const SDL_Point& cursorPosition) {
            onSpriteDragReleased(i, cursorPosition);
        });

        // Darken every 5th frame and add a number above it.
        if (i % 5 == 0) {
            frame->drawDarkBackground = true;

            auto numberText{std::make_unique<AUI::Text>(SDL_Rect{0, 0, 24, 18},
                                                        "NumberText")};
            styleNumberText(*numberText, std::to_string(i));
            numberContainer.push_back(std::move(numberText));
        }

        frameContainer.push_back(std::move(frame));
    }

    // Fill the frames that contain sprites.
    for (const EditorAnimation::Frame& frame : activeAnimation->frames) {
        AM_ASSERT(frame.frameNumber < frameContainer.size(),
                  "Invalid frame number.");
        TimelineFrame& timelineFrame{
            static_cast<TimelineFrame&>(*frameContainer[frame.frameNumber])};
        timelineFrame.hasSprite = true;
    }

    // Update this widget's width to match the frames, adding an extra frame's
    // width for the loop handle.
    SDL_Rect newLogicalExtent{logicalExtent};
    newLogicalExtent.w
        = activeAnimation->frameCount * TimelineFrame::LOGICAL_WIDTH;
    newLogicalExtent.w += TimelineFrame::LOGICAL_WIDTH;
    setLogicalExtent(newLogicalExtent);

    // Any time we update the frames, we want to stop playing the old animation
    // (so we're forced to reset the play state).
    playingAnimation = false;
}

void AnimationTimeline::setFrameScrubberPosition(Uint8 frameNumber)
{
    // Set the new frame number.
    selectedFrameNumber = frameNumber;

    // Center the scrubber over the new frame.
    AM_ASSERT(frameNumber < frameContainer.size(), "Invalid frame number.");

    SDL_Rect newScrubberExtent{frameScrubber.getLogicalExtent()};
    newScrubberExtent.x = TimelineFrame::LOGICAL_WIDTH * frameNumber;
    frameScrubber.setLogicalExtent(newScrubberExtent);

    // If the user set a callback, call it.
    if (onSelectionChanged) {
        onSelectionChanged(selectedFrameNumber);
    }
}

void AnimationTimeline::setLoopHandlePosition(Uint8 frameNumber)
{
    // Center the handle over the new frame.
    AM_ASSERT(frameNumber <= frameContainer.size(), "Invalid frame number.");

    SDL_Rect newHandleExtent{loopHandle.getLogicalExtent()};
    newHandleExtent.x = TimelineFrame::LOGICAL_WIDTH * frameNumber;
    loopHandle.setLogicalExtent(newHandleExtent);
}

Uint8 AnimationTimeline::getCursorFrame(const SDL_Point& cursorPosition)
{
    // Offset the cursor to be relative to this widget's top left.
    SDL_Point offsetCursorPosition{cursorPosition};
    offsetCursorPosition.x -= clippedExtent.x;
    offsetCursorPosition.y -= clippedExtent.y;

    // Calculate which frame the cursor is aligned with.
    int cursorFrame{
        AUI::ScalingHelpers::actualToLogical(offsetCursorPosition.x)};
    cursorFrame /= TimelineFrame::LOGICAL_WIDTH;

    // Clamp to valid values.
    cursorFrame = std::clamp(cursorFrame, 0,
                             static_cast<int>(frameContainer.size()));

    return static_cast<Uint8>(cursorFrame);
}

void AnimationTimeline::styleNumberText(AUI::Text& textObject,
                                        const std::string& text)
{
    textObject.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 16);
    textObject.setColor({255, 255, 255, 255});
    textObject.setHorizontalAlignment(AUI::Text::HorizontalAlignment::Center);
    textObject.setText(text);
}

} // End namespace ResourceImporter
} // End namespace AM
