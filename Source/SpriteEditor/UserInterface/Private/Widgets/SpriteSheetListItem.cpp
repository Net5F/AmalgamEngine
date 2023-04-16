#include "SpriteSheetListItem.h"
#include "Paths.h"
#include "Ignore.h"
#include "AUI/ScalingHelpers.h"
#include "AUI/SDLHelpers.h"

namespace AM
{
namespace SpriteEditor
{
SpriteSheetListItem::SpriteSheetListItem(const std::string& inHeaderText,
                         const std::string& inDebugName)
: LibraryListItem("", inDebugName)
// Note: We use our child's text and leave our parent ListItem's blank, so 
//       that all of the text and arrows are above our highlight images.
, spriteListItemContainer(inHeaderText)
{
    // Add our children so they're included in rendering, etc.
    children.push_back(spriteListItemContainer);

    // Set our padding.
    spriteListItemContainer.setLeftPadding(32);

    // Set our child container's click region to cover from the far left, to 
    // the start of our text.
    SDL_Rect textLogicalExtent{spriteListItemContainer.headerText.getLogicalExtent()};
    spriteListItemContainer.setClickRegionLogicalExtent(
        {0, 0, textLogicalExtent.x, textLogicalExtent.h});

    // If our child container expands or collapses, adjust our height to match.
    spriteListItemContainer.setOnHeightChanged([this]() {
        logicalExtent.h = spriteListItemContainer.getLogicalExtent().h;
    });
}

AUI::EventResult
    SpriteSheetListItem::onMouseDown(AUI::MouseButtonType buttonType,
                                      const SDL_Point& cursorPosition)
{
    // Only respond to the left mouse button.
    if (buttonType != AUI::MouseButtonType::Left) {
        return AUI::EventResult{.wasHandled{false}};
    }

    // If we aren't selected and our text was clicked, select this widget.
    if (!isSelected
        && AUI::SDLHelpers::pointInRect(cursorPosition,
                                        text.getClippedExtent())) {
        select();

        return AUI::EventResult{.wasHandled{true}};
    }

    // Note: If our arrow was clicked, it'll be handled when this propagates 
    //       to our child collapsible container.
    return AUI::EventResult{.wasHandled{false}};
}

AUI::EventResult SpriteSheetListItem::onMouseDoubleClick(AUI::MouseButtonType buttonType,
                                          const SDL_Point& cursorPosition)
{
    // We treat additional clicks as regular MouseDown events.
    return onMouseDown(buttonType, cursorPosition);
}

AUI::EventResult SpriteSheetListItem::onMouseMove(const SDL_Point& cursorPosition)
{
    // If we're selected, don't change to hovered.
    if (isSelected) {
        return AUI::EventResult{.wasHandled{false}};
    }

    // If the mouse is within the header extent.
    SDL_Rect headerExtent{spriteListItemContainer.getHeaderExtent()};
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

void SpriteSheetListItem::onMouseEnter()
{
    // We handle this in onMouseMove(), but we override this to stop 
    // LibraryListItem's behavior.
}

void SpriteSheetListItem::onMouseLeave()
{
    // If we're hovered, unhover.
    if (isHovered) {
        setIsHovered(false);
    }
}

} // End namespace SpriteEditor
} // End namespace AM
