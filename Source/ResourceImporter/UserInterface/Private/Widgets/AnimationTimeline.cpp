#include "AnimationTimeline.h"
#include "EditorAnimation.h"
#include "TimelineFrame.h"
#include "Paths.h"
#include "AMAssert.h"
#include "AUI/ScalingHelpers.h"
#include "Log.h"
#include <algorithm>

namespace AM
{
namespace ResourceImporter
{
AnimationTimeline::AnimationTimeline(const SDL_Rect& inLogicalExtent,
                                     const std::string& inDebugName)
: AUI::Widget(inLogicalExtent, inDebugName)
, numberContainer{{0, 4, logicalExtent.w, 18}, "NumberContainer"}
, frameContainer{{0, 24, logicalExtent.w, 26}, "FrameContainer"}
, scrubber{}
, selectedFrameNumber{0}
, activeAnimation{nullptr}
, originSpriteDragFrameIndex{0}
, currentSpriteDragFrameIndex{0}
, animationAccumulator{0}
, playingAnimation{false}
{
    // Add our children so they're included in rendering, etc.
    children.push_back(numberContainer);
    children.push_back(frameContainer);
    children.push_back(scrubber);

    /* Numbers. */
    numberContainer.setNumRows(1);
    numberContainer.setCellWidth(24 + 96);
    numberContainer.setCellHeight(18);
    numberContainer.setScrollingEnabled(false);

    /* Frame container. */
    frameContainer.setNumRows(1);
    frameContainer.setCellWidth(24);
    frameContainer.setCellHeight(26);
    frameContainer.setScrollingEnabled(false);

    scrubber.setOnDragged([&](const SDL_Point& cursorPosition) {
        onScrubberDragged(cursorPosition);
    });
}

void AnimationTimeline::setActiveAnimation(
    const EditorAnimation& newActiveAnimation)
{
    activeAnimation = &newActiveAnimation;

    refreshFrames();

    setSelectedFrame(selectedFrameNumber);
}

void AnimationTimeline::setFrameCount(Uint8 newFrameCount)
{
    refreshFrames();

    setSelectedFrame(selectedFrameNumber);
}

void AnimationTimeline::setFrame(Uint8 frameNumber, bool hasSprite)
{
    TimelineFrame& frame{static_cast<TimelineFrame&>(*frameContainer[frameNumber])};
    frame.hasSprite = hasSprite;

    // If the frame is currently selected, refresh it so the user gets notified 
    // of the sprite change.
    if (frameNumber == selectedFrameNumber) {
        setSelectedFrame(selectedFrameNumber);
    }
}

void AnimationTimeline::playAnimation()
{
    setSelectedFrame(0);
    animationTimer.reset();
    animationAccumulator = 0;
    playingAnimation = true;
}

Uint8 AnimationTimeline::getSelectedFrameNumber()
{
    return selectedFrameNumber;
}

void AnimationTimeline::setOnSelectionChanged(
    std::function<void(Uint8)> inOnSelectionChanged)
{
    onSelectionChanged = std::move(inOnSelectionChanged);
}


void AnimationTimeline::setOnSpriteMoved(
    std::function<void(Uint8, Uint8)> inOnSpriteMoved)
{
    onSpriteMoved = std::move(inOnSpriteMoved);
}

void AnimationTimeline::onTick(double)
{
    // If we're playing an animation and enough time has passed, go to the 
    // next frame.
    if (playingAnimation) {
        animationAccumulator += animationTimer.getTimeAndReset();

        double animationTimestepS{1
                                  / static_cast<double>(activeAnimation->fps)};
        while (animationAccumulator > animationTimestepS) {
            setSelectedFrame(selectedFrameNumber + 1);

            animationAccumulator -= animationTimestepS;

            // If we've hit the last frame, stop playing the animation.
            if (selectedFrameNumber == (activeAnimation->frameCount - 1)) {
                playingAnimation = false;
                return;
            }
        }
    }
}

void AnimationTimeline::onScrubberDragged(const SDL_Point& cursorPosition)
{
    // Ignore scrubber interactions if we're playing an animation.
    if (playingAnimation) {
        return;
    }

    Uint8 cursorFrame{getCursorFrame(cursorPosition)};

    // If the cursor isn't aligned with the current selected frame, select the 
    // new one.
    if ((selectedFrameNumber != cursorFrame)
        && (cursorFrame < frameContainer.size())) {
        setSelectedFrame(cursorFrame);
    }
}

void AnimationTimeline::onSpriteDragStarted(Uint8 frameIndex,
                                            const SDL_Point& cursorPosition)
{
    // Remember where the drag started.
    originSpriteDragFrameIndex = frameIndex;
    currentSpriteDragFrameIndex = frameIndex;
}

void AnimationTimeline::onSpriteDragged(Uint8 frameIndex,
                                        const SDL_Point& cursorPosition)
{
    // If the cursor is over a new frame, reset the old frame's circle 
    // and add one to the new frame.
    Uint8 cursorFrame{getCursorFrame(cursorPosition)};
    if (cursorFrame != currentSpriteDragFrameIndex) {
        TimelineFrame& oldCurrentFrame{static_cast<TimelineFrame&>(
            *frameContainer[currentSpriteDragFrameIndex])};
        TimelineFrame& newCurrentFrame{static_cast<TimelineFrame&>(
            *frameContainer[cursorFrame])};

        // If we're moving from the origin frame, remove its marker.
        if (currentSpriteDragFrameIndex == originSpriteDragFrameIndex) {
            oldCurrentFrame.hasSprite = false;
        }
        else {
            // Not the origin frame, return it to whatever it was.
            oldCurrentFrame.hasSprite = (activeAnimation->getSpriteAtFrame(
                                             currentSpriteDragFrameIndex)
                                         != nullptr);
        }
        newCurrentFrame.hasSprite = true;

        currentSpriteDragFrameIndex = cursorFrame;
    }
}

void AnimationTimeline::onSpriteDragReleased(Uint8 frameIndex,
                                             const SDL_Point& cursorPosition)
{
    // Reset the display state.
    TimelineFrame& originFrame{static_cast<TimelineFrame&>(
        *frameContainer[originSpriteDragFrameIndex])};
    TimelineFrame& currentFrame{static_cast<TimelineFrame&>(
        *frameContainer[currentSpriteDragFrameIndex])};

    bool originHasSprite{originFrame.hasSprite};
    bool currentHasSprite{currentFrame.hasSprite};
    originFrame.hasSprite = currentHasSprite;
    currentFrame.hasSprite = originHasSprite;

    // If the cursor is over a new frame, move the sprite to it.
    if ((currentSpriteDragFrameIndex != originSpriteDragFrameIndex)
        && onSpriteMoved) {
        onSpriteMoved(originSpriteDragFrameIndex, currentSpriteDragFrameIndex);
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
        frame->setOnPressed([&, i]() { setSelectedFrame(i); });
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

    // Fill the frames that contain frames.
    for (const EditorAnimation::Frame& frame : activeAnimation->frames) {
        AM_ASSERT(frame.frameNumber < frameContainer.size(),
                  "Invalid frame index.");
        TimelineFrame& timelineFrame{
            static_cast<TimelineFrame&>(*frameContainer[frame.frameNumber])};
        timelineFrame.hasSprite = true;
    }

    // Update this widget's width to match the frames.
    SDL_Rect newLogicalExtent{logicalExtent};
    newLogicalExtent.w
        = activeAnimation->frameCount * TimelineFrame::LOGICAL_WIDTH;
    setLogicalExtent(newLogicalExtent);
}

void AnimationTimeline::setSelectedFrame(Uint8 frameNumber)
{
    // Set the new frame number.
    selectedFrameNumber = frameNumber;

    // Center the scrubber over the new frame.
    AM_ASSERT(frameNumber < frameContainer.size(), "Invalid frame number.");

    SDL_Rect newScrubberExtent{scrubber.getLogicalExtent()};
    newScrubberExtent.x = TimelineFrame::LOGICAL_WIDTH * frameNumber;
    scrubber.setLogicalExtent(newScrubberExtent);

    // If the user set a callback, call it.
    if (onSelectionChanged) {
        onSelectionChanged(selectedFrameNumber);
    }
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
                             static_cast<int>(frameContainer.size() - 1));

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
