#include "LibraryListItem.h"
#include "Paths.h"
#include "AUI/Text.h"

namespace AM
{
namespace ResourceImporter
{
LibraryListItem::LibraryListItem(const std::string& inText,
                                 const std::string& inDebugName)
: AUI::Widget({0, 0, 318, 30}, inDebugName)
, type{}
, ID{0}
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
    hoveredImage.setSimpleImage(Paths::TEXTURE_DIR + "Highlights/Hovered.png");
    selectedImage.setSimpleImage(Paths::TEXTURE_DIR
                                 + "Highlights/Selected.png");

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
    if (onSelected) {
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
    if (onDeselected) {
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

void LibraryListItem::setOnSelected(
    std::function<void(LibraryListItem*)> inOnSelected)
{
    onSelected = std::move(inOnSelected);
}

void LibraryListItem::setOnDeselected(
    std::function<void(LibraryListItem*)> inOnDeselected)
{
    onDeselected = std::move(inOnDeselected);
}

void LibraryListItem::setOnActivated(
    std::function<void(LibraryListItem*)> inOnActivated)
{
    onActivated = std::move(inOnActivated);
}

AUI::EventResult LibraryListItem::onMouseDown(AUI::MouseButtonType buttonType,
                                              const SDL_Point&)
{
    // Only respond to the left mouse button.
    if (buttonType != AUI::MouseButtonType::Left) {
        return AUI::EventResult{.wasHandled{false}};
    }

    // If we aren't already selected, select this widget.
    if (!isSelected) {
        select();
    }
    // If we're already selected and the ctrl key is held, deselect.
    else {
        const Uint8* keyStates{SDL_GetKeyboardState(nullptr)};
        if (keyStates[SDL_SCANCODE_LCTRL] || keyStates[SDL_SCANCODE_RCTRL]) {
            setIsSelected(false);
            if (onDeselected) {
                onDeselected(this);
            }
        }
    }

    return AUI::EventResult{.wasHandled{true}};
}

AUI::EventResult
    LibraryListItem::onMouseDoubleClick(AUI::MouseButtonType buttonType,
                                        const SDL_Point&)
{
    // Only respond to the left mouse button.
    if (buttonType != AUI::MouseButtonType::Left) {
        return AUI::EventResult{.wasHandled{false}};
    }

    // If we aren't already selected, select this widget.
    if (!isSelected) {
        select();
    }
    // If we're already selected and the ctrl key is held, deselect.
    else {
        const Uint8* keyStates{SDL_GetKeyboardState(nullptr)};
        if (keyStates[SDL_SCANCODE_LCTRL] || keyStates[SDL_SCANCODE_RCTRL]) {
            setIsSelected(false);
            if (onDeselected) {
                onDeselected(this);
            }

            return AUI::EventResult{.wasHandled{true}};
        }
    }

    // If the user set a callback for this event, call it.
    // Note: We don't activate if shift or ctrl is held, since the user is 
    //       likely trying to quickly select multiple items.
    if (onActivated && !modifierKeyIsHeld()) {
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

bool LibraryListItem::modifierKeyIsHeld()
{
    const Uint8* keyStates{SDL_GetKeyboardState(nullptr)};
    bool shiftIsHeld{keyStates[SDL_SCANCODE_LSHIFT]
                     || keyStates[SDL_SCANCODE_RSHIFT]};
    bool ctrlIsHeld{keyStates[SDL_SCANCODE_LCTRL]
                    || keyStates[SDL_SCANCODE_RCTRL]};

    return shiftIsHeld || ctrlIsHeld;
}

} // End namespace ResourceImporter
} // End namespace AM
