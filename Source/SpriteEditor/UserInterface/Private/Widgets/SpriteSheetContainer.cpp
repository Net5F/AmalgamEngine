#include "SpriteSheetContainer.h"
#include "Paths.h"
#include "Ignore.h"
#include "AUI/ScalingHelpers.h"
#include "AUI/SDLHelpers.h"

namespace AM
{
namespace SpriteEditor
{
SpriteSheetContainer::SpriteSheetContainer(const std::string& inHeaderText,
                         const std::string& inDebugName)
: CategoryContainer(inHeaderText, inDebugName)
, isHovered{false}
, isSelected{false}
, hoveredImage({0, 0, logicalExtent.w, logicalExtent.h})
, selectedImage({0, 0, logicalExtent.w, logicalExtent.h})
, clickableExtent{
      AUI::ScalingHelpers::logicalToActual({0, 0, 56, logicalExtent.h})}
{
    // Add our children so they're included in rendering, etc.
    // Note: We insert these between expandedImage/collapsedImage and headerText.
    children.insert(children.begin() + 2, hoveredImage);
    children.insert(children.begin() + 3, selectedImage);

    // Add our backgrounds.
    hoveredImage.setSimpleImage(Paths::TEXTURE_DIR + "LibraryWindow/ListItemHovered.png");
    selectedImage.setSimpleImage(Paths::TEXTURE_DIR + "LibraryWindow/ListItemSelected.png");

    // Set our padding.
    setLeftPadding(32);

    // Make the images we aren't using invisible.
    hoveredImage.setIsVisible(false);
    selectedImage.setIsVisible(false);
}

void SpriteSheetContainer::select()
{
    // If we are already selected, do nothing.
    if (isSelected) {
        return;
    }

    // Flag that we're now selected.
    setIsSelected(true);

    // If the user set a callback for this event, call it.
    if (onSelected != nullptr) {
        onSelected(this);
    }
}

void SpriteSheetContainer::deselect()
{
    // If we aren't selected, do nothing.
    if (!isSelected) {
        return;
    }

    // Flag that we're not selected.
    setIsSelected(false);

    // If the user set a callback for this event, call it.
    if (onDeselected != nullptr) {
        onDeselected(this);
    }
}

bool SpriteSheetContainer::getIsSelected() const
{
    return isSelected;
}

void SpriteSheetContainer::setOnSelected(std::function<void(SpriteSheetContainer*)> inOnSelected)
{
    onSelected = std::move(inOnSelected);
}

void SpriteSheetContainer::setOnDeselected(std::function<void(SpriteSheetContainer*)> inOnDeselected)
{
    onDeselected = std::move(inOnDeselected);
}

AUI::EventResult
    SpriteSheetContainer::onMouseDown(AUI::MouseButtonType buttonType,
                                      const SDL_Point& cursorPosition)
{
    // Only respond to the left mouse button.
    if (buttonType != AUI::MouseButtonType::Left) {
        return AUI::EventResult{.wasHandled{false}};
    }

    // If our collapse/expand arrow was clicked, pass the event to 
    // CollapsibleContainer's handler.
    SDL_Point relativeCursorPosition{(cursorPosition.x - clippedExtent.x),
                                     (cursorPosition.y - clippedExtent.y)};
    if (AUI::SDLHelpers::pointInRect(relativeCursorPosition, clickableExtent)) {
        return CollapsibleContainer::onMouseDown(buttonType, cursorPosition);
    }
    else if (!isSelected) {
        // A different part of our widget was clicked. Check if it was the 
        // header.

        // Calculate the header's extent (we can't just use clippedExtent 
        // because when we're expanded it accounts for the whole container).
        SDL_Rect headerExtent{
            AUI::ScalingHelpers::logicalToActual(headerLogicalExtent)};
        headerExtent.x = clippedExtent.x;
        headerExtent.y = clippedExtent.y;

        // If the header was clicked, select this widget.
        if (AUI::SDLHelpers::pointInRect(cursorPosition, headerExtent)) {
            select();

            return AUI::EventResult{.wasHandled{true}};
        }
    }

    return AUI::EventResult{.wasHandled{false}};
}

AUI::EventResult SpriteSheetContainer::onMouseDoubleClick(AUI::MouseButtonType buttonType,
                                          const SDL_Point& cursorPosition)
{
    // We treat additional clicks as regular MouseDown events.
    return onMouseDown(buttonType, cursorPosition);
}

AUI::EventResult SpriteSheetContainer::onMouseMove(const SDL_Point& cursorPosition)
{
    // If we're selected, don't change to hovered.
    if (isSelected) {
        return AUI::EventResult{.wasHandled{false}};
    }

    // Calculate the header's extent (we can't just use clippedExtent because 
    // when we're expanded it accounts for the whole container).
    SDL_Rect headerExtent{
        AUI::ScalingHelpers::logicalToActual(headerLogicalExtent)};
    headerExtent.x = clippedExtent.x;
    headerExtent.y = clippedExtent.y;

    // If the mouse is within the header extent.
    if (AUI::SDLHelpers::pointInRect(cursorPosition, headerExtent)) {
        // If we're not hovered, become hovered.
        if (!isHovered) {
            setIsHovered(true);
        }

        return AUI::EventResult{.wasHandled{true}};
    }
    else if (isHovered) {
        // We're hovered, unhover.
        setIsHovered(false);
    }

    return AUI::EventResult{.wasHandled{false}};
}

void SpriteSheetContainer::onMouseLeave()
{
    // If we're hovered, unhover.
    if (isHovered) {
        setIsHovered(false);
    }
}

void SpriteSheetContainer::setIsHovered(bool inIsHovered)
{
    isHovered = inIsHovered;
    hoveredImage.setIsVisible(isHovered);
}

void SpriteSheetContainer::setIsSelected(bool inIsSelected)
{
    isSelected = inIsSelected;
    selectedImage.setIsVisible(isSelected);
}

} // End namespace SpriteEditor
} // End namespace AM
