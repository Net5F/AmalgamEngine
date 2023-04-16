#include "LibraryListItem.h"
#include "Paths.h"
#include "AUI/Text.h"
#include "Ignore.h"

namespace AM
{
namespace SpriteEditor
{
LibraryListItem::LibraryListItem(const std::string& inText,
                         const std::string& inDebugName)
: AUI::Widget({0, 0, 318, 30}, inDebugName)
, hoveredImage({0, 0, logicalExtent.w, logicalExtent.h})
, selectedImage({0, 0, logicalExtent.w, logicalExtent.h})
, text({0, 0, logicalExtent.w, logicalExtent.h})
, isHovered{false}
, isSelected{false}
{
    // Add our children so they're included in rendering, etc.
    children.push_back(hoveredImage);
    children.push_back(selectedImage);
    children.push_back(text);

    // Add our backgrounds.
    hoveredImage.setSimpleImage(Paths::TEXTURE_DIR + "LibraryWindow/ListItemHovered.png");
    selectedImage.setSimpleImage(Paths::TEXTURE_DIR + "LibraryWindow/ListItemSelected.png");

    // Set our text properties.
    text.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
    text.setColor({255, 255, 255, 255});
    text.setVerticalAlignment(AUI::Text::VerticalAlignment::Center);
    text.setText(inText);

    // Make the images we aren't using invisible.
    hoveredImage.setIsVisible(false);
    selectedImage.setIsVisible(false);
}

void LibraryListItem::select()
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

void LibraryListItem::deselect()
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

bool LibraryListItem::getIsHovered() const
{
    return isHovered;
}

bool LibraryListItem::getIsSelected() const
{
    return isSelected;
}

void LibraryListItem::setLeftPadding(int inLeftPadding)
{
    text.setLogicalExtent(
        {inLeftPadding, 0, (logicalExtent.w - inLeftPadding), logicalExtent.h});
}

void LibraryListItem::setOnSelected(std::function<void(LibraryListItem*)> inOnSelected)
{
    onSelected = std::move(inOnSelected);
}

void LibraryListItem::setOnDeselected(std::function<void(LibraryListItem*)> inOnDeselected)
{
    onDeselected = std::move(inOnDeselected);
}

void LibraryListItem::setOnActivated(std::function<void(LibraryListItem*)> inOnActivated)
{
    onActivated = std::move(inOnActivated);
}

AUI::EventResult LibraryListItem::onMouseDown(AUI::MouseButtonType buttonType,
                                   const SDL_Point& cursorPosition)
{
    ignore(cursorPosition);

    // Only respond to the left mouse button.
    if (buttonType != AUI::MouseButtonType::Left) {
        return AUI::EventResult{.wasHandled{false}};
    }
    // If we're already selected, do nothing.
    else if (isSelected) {
        return AUI::EventResult{.wasHandled{false}};
    }

    if (!isSelected) {
        // This was a single click. If we aren't already selected, select
        // this widget.
        select();

        // Note: It would make sense to request focus and deselect when we
        //       lose focus, but it seems like every use case for "select a
        //       thumbnail" prefers leaving the thumbnail selected and
        //       controlling it from the outside.
        //       E.g. for build mode, we want the thumbnail to stay selected
        //       until the parent tells it to deselect.

        return AUI::EventResult{.wasHandled{true}};
    }

    return AUI::EventResult{.wasHandled{false}};
}

AUI::EventResult LibraryListItem::onMouseDoubleClick(AUI::MouseButtonType buttonType,
                                          const SDL_Point& cursorPosition)
{
    ignore(cursorPosition);

    // Only respond to the left mouse button.
    if (buttonType != AUI::MouseButtonType::Left) {
        return AUI::EventResult{.wasHandled{false}};
    }

    // If the user set a callback for this event, call it.
    if (onActivated != nullptr) {
        onActivated(this);
    }

    return AUI::EventResult{.wasHandled{true}};
}

void LibraryListItem::onMouseEnter()
{
    // If we're selected, ignore hovers.
    if (isSelected) {
        return;
    }

    // If we're not hovered, become hovered.
    if (!isHovered) {
        setIsHovered(true);
    }
    else {
        // We're hovered, unhover.
        setIsHovered(false);
    }
}

void LibraryListItem::onMouseLeave()
{
    // If we're hovered, unhover.
    if (isHovered) {
        setIsHovered(false);
    }
}

void LibraryListItem::setIsHovered(bool inIsHovered)
{
    isHovered = inIsHovered;
    hoveredImage.setIsVisible(isHovered);
}

void LibraryListItem::setIsSelected(bool inIsSelected)
{
    isSelected = inIsSelected;
    selectedImage.setIsVisible(isSelected);
}

} // End namespace SpriteEditor
} // End namespace AM
