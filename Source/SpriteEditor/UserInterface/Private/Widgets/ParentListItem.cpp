#include "ParentListItem.h"
#include "Paths.h"
#include "Ignore.h"
#include "AUI/ScalingHelpers.h"
#include "AUI/SDLHelpers.h"

namespace AM
{
namespace SpriteEditor
{
ParentListItem::ParentListItem(const std::string& inHeaderText,
                         const std::string& inDebugName)
: LibraryListItem("", inDebugName)
// Note: We use our child's text and leave our parent ListItem's blank, so 
//       that all of the text and arrows are above our highlight images.
, childListItemContainer(inHeaderText)
{
    // Add our children so they're included in rendering, etc.
    children.push_back(childListItemContainer);

    // Set our padding.
    childListItemContainer.setLeftPadding(32);

    // Set our child container's click region to cover from the far left, to 
    // the start of our text.
    SDL_Rect textLogicalExtent{childListItemContainer.headerText.getLogicalExtent()};
    childListItemContainer.setClickRegionLogicalExtent(
        {0, 0, textLogicalExtent.x, textLogicalExtent.h});

    // If our child container expands or collapses, adjust our height to match.
    childListItemContainer.setOnHeightChanged([this]() {
        logicalExtent.h = childListItemContainer.getLogicalExtent().h;
    });
}

AUI::EventResult
    ParentListItem::onMouseDown(AUI::MouseButtonType buttonType,
                                      const SDL_Point& cursorPosition)
{
    // Only respond to the left mouse button.
    if (buttonType != AUI::MouseButtonType::Left) {
        return AUI::EventResult{.wasHandled{false}};
    }

    // If our text was clicked, select this widget.
    if (AUI::SDLHelpers::pointInRect(cursorPosition, text.getClippedExtent())) {
        // If we're already selected, do nothing.
        if (!isSelected) {
            select();
        }

        return AUI::EventResult{.wasHandled{true}};
    }

    // Note: If our arrow was clicked, it'll be handled when this propagates 
    //       to our child collapsible container.
    return AUI::EventResult{.wasHandled{false}};
}

AUI::EventResult ParentListItem::onMouseDoubleClick(AUI::MouseButtonType buttonType,
                                          const SDL_Point& cursorPosition)
{
    // We treat additional clicks as regular MouseDown events.
    return onMouseDown(buttonType, cursorPosition);
}

AUI::EventResult ParentListItem::onMouseMove(const SDL_Point& cursorPosition)
{
    // If the mouse is within the header extent.
    SDL_Rect headerExtent{childListItemContainer.getHeaderExtent()};
    if (AUI::SDLHelpers::pointInRect(cursorPosition, headerExtent)) {
        // If we're not selected or hovered, become hovered.
        if (!isSelected && !isHovered) {
            setIsHovered(true);
        }

        return AUI::EventResult{.wasHandled{true}};
    }
    else if (isHovered) {
        // We're hovered and the mouse is outside the header, unhover.
        setIsHovered(false);
    }

    return AUI::EventResult{.wasHandled{false}};
}

void ParentListItem::onMouseEnter()
{
    // We handle this in onMouseMove(), but we override this to stop 
    // LibraryListItem's behavior.
}

void ParentListItem::onMouseLeave()
{
    // If we're hovered, unhover.
    if (isHovered) {
        setIsHovered(false);
    }
}

} // End namespace SpriteEditor
} // End namespace AM
