#include "AnimationTimeline.h"
#include "EditorAnimation.h"
#include "TimelineCell.h"
#include "Paths.h"
#include "AMAssert.h"
#include "AUI/ScalingHelpers.h"
#include "Log.h"

namespace AM
{
namespace ResourceImporter
{
AnimationTimeline::AnimationTimeline(const SDL_Rect& inLogicalExtent,
                                     const std::string& inDebugName)
: AUI::Widget(inLogicalExtent, inDebugName)
, numberContainer{{0, 4, logicalExtent.w, 18}, "NumberContainer"}
, cellContainer{{0, 24, logicalExtent.w, 26}, "CellContainer"}
, scrubber{}
, selectedFrameNumber{0}
, activeAnimation{nullptr}
{
    // Add our children so they're included in rendering, etc.
    children.push_back(numberContainer);
    children.push_back(cellContainer);
    children.push_back(scrubber);

    /* Numbers. */
    numberContainer.setNumRows(1);
    numberContainer.setCellWidth(24 + 96);
    numberContainer.setCellHeight(18);
    numberContainer.setScrollingEnabled(false);

    /* Cell container. */
    cellContainer.setNumRows(1);
    cellContainer.setCellWidth(24);
    cellContainer.setCellHeight(26);
    cellContainer.setScrollingEnabled(false);

    scrubber.setOnDragged([&](const SDL_Point& cursorPosition) {
        onScrubberDragged(cursorPosition);
    });
}

void AnimationTimeline::setActiveAnimation(
    const EditorAnimation& newActiveAnimation)
{
    activeAnimation = &newActiveAnimation;

    refreshCells();

    setSelectedFrame(selectedFrameNumber);
}

void AnimationTimeline::setFrameCount(Uint8 newFrameCount)
{
    refreshCells();

    setSelectedFrame(selectedFrameNumber);
}

void AnimationTimeline::setFrame(Uint8 frameNumber, bool hasSprite)
{
    TimelineCell& cell{static_cast<TimelineCell&>(*cellContainer[frameNumber])};
    cell.hasSprite = hasSprite;

    // If the frame is currently selected, refresh it so the user gets notified 
    // of the sprite change.
    if (frameNumber == selectedFrameNumber) {
        setSelectedFrame(selectedFrameNumber);
    }
}

Uint8 AnimationTimeline::getSelectedFrameNumber()
{
    return selectedFrameNumber;
}

void AnimationTimeline::setOnSelectionChanged(
    std::function<void(const EditorSprite*)> inOnSelectionChanged)
{
    onSelectionChanged = std::move(inOnSelectionChanged);
}

void AnimationTimeline::onScrubberDragged(const SDL_Point& cursorPosition)
{
    // Offset the cursor to be relative to this widget's top left.
    SDL_Point offsetCursorPosition{cursorPosition};
    offsetCursorPosition.x -= clippedExtent.x;
    offsetCursorPosition.y -= clippedExtent.y;

    // Calculate which cell the cursor is aligned with.
    int cursorCell{
        AUI::ScalingHelpers::actualToLogical(offsetCursorPosition.x)};
    cursorCell /= TimelineCell::LOGICAL_WIDTH;

    // If the cursor isn't aligned with the current cell, select the new one.
    if ((selectedFrameNumber != cursorCell)
        && (cursorCell < cellContainer.size())) {
        setSelectedFrame(cursorCell);
    }
}

void AnimationTimeline::refreshCells()
{
    // Fill the cell container with empty cells to match the animation's frame 
    // count.
    cellContainer.clear();
    numberContainer.clear();
    for (int i = 0; i < activeAnimation->frameCount; ++i) {
        auto cell{std::make_unique<TimelineCell>()};
        cell->setOnPressed([&, i]() { setSelectedFrame(i); });

        // Darken every 5th cell and add a number above it.
        if (i % 5 == 0) {
            cell->drawDarkBackground = true;

            auto numberText{std::make_unique<AUI::Text>(SDL_Rect{0, 0, 24, 18},
                                                        "NumberText")};
            styleNumberText(*numberText, std::to_string(i));
            numberContainer.push_back(std::move(numberText));
        }

        cellContainer.push_back(std::move(cell));
    }

    // Fill the cells that contain frames.
    for (const EditorAnimation::Frame& frame : activeAnimation->frames) {
        AM_ASSERT(frame.frameNumber < cellContainer.size(),
                  "Invalid cell index.");
        TimelineCell& cell{
            static_cast<TimelineCell&>(*cellContainer[frame.frameNumber])};
        cell.hasSprite = true;
    }

    // Update this widget's width to match the cells.
    SDL_Rect newLogicalExtent{logicalExtent};
    newLogicalExtent.w
        = activeAnimation->frameCount * TimelineCell::LOGICAL_WIDTH;
    setLogicalExtent(newLogicalExtent);
}

void AnimationTimeline::setSelectedFrame(Uint8 frameNumber)
{
    // Set the new frame number.
    selectedFrameNumber = frameNumber;

    // Center the scrubber over the new frame.
    AM_ASSERT(frameNumber < cellContainer.size(), "Invalid frame number.");

    SDL_Rect newScrubberExtent{scrubber.getLogicalExtent()};
    newScrubberExtent.x = TimelineCell::LOGICAL_WIDTH * frameNumber;
    scrubber.setLogicalExtent(newScrubberExtent);

    // If the user set a callback, call it.
    if (onSelectionChanged) {
        // If this cell contains a frame, pass it to the user.
        const auto& frames{activeAnimation->frames};
        for (const EditorAnimation::Frame& frame : frames) {
            if (frame.frameNumber == selectedFrameNumber) {
                onSelectionChanged(&(frame.sprite.get()));
                return;
            }
        }

        // No frame in this cell. Pass nullptr.
        onSelectionChanged(nullptr);
    }
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
