#include "ParentListItem.h"
#include "Paths.h"
#include "AUI/ScalingHelpers.h"
#include <SDL_rect.h>

namespace AM
{
namespace ResourceImporter
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
    SDL_Rect textLogicalExtent{
        childListItemContainer.headerText.getLogicalExtent()};
    childListItemContainer.setClickRegionLogicalExtent(
        {0, 0, textLogicalExtent.x, textLogicalExtent.h});
}

AUI::EventResult ParentListItem::onMouseDown(AUI::MouseButtonType buttonType,
                                             const SDL_Point& cursorPosition)
{
    // Only respond to the left mouse button.
    if (buttonType != AUI::MouseButtonType::Left) {
        return AUI::EventResult{.wasHandled{false}};
    }

    // If our text was clicked, select this widget.
    if (SDL_PointInRect(&cursorPosition, &(text.getClippedExtent()))) {
        // Note: Our only difference in behavior from LibraryListItem is that 
        //       we only react to clicking on the text region, so we can just 
        //       call its handler.
        LibraryListItem::onMouseDown(buttonType, cursorPosition);
        return AUI::EventResult{.wasHandled{true}};
    }

    // Note: If our arrow was clicked, it'll be handled when this propagates
    //       to our child collapsible container.
    return AUI::EventResult{.wasHandled{false}};
}

AUI::EventResult
    ParentListItem::onMouseDoubleClick(AUI::MouseButtonType buttonType,
                                       const SDL_Point& cursorPosition)
{
    // Only respond to the left mouse button.
    if (buttonType != AUI::MouseButtonType::Left) {
        return AUI::EventResult{.wasHandled{false}};
    }

    // If our text was clicked, select this widget.
    if (SDL_PointInRect(&cursorPosition, &(text.getClippedExtent()))) {
        // Note: Our only difference in behavior from LibraryListItem is that 
        //       we only react to clicking on the text region, so we can just 
        //       call its handler.
        LibraryListItem::onMouseDoubleClick(buttonType, cursorPosition);
    }

    // Note: If our arrow was clicked, it'll be handled when this propagates
    //       to our child collapsible container.
    return AUI::EventResult{.wasHandled{false}};
}

AUI::EventResult ParentListItem::onMouseMove(const SDL_Point& cursorPosition)
{
    // If the mouse is within the header extent.
    SDL_Rect headerExtent{childListItemContainer.getHeaderExtent()};
    if (SDL_PointInRect(&cursorPosition, &headerExtent)) {
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

void ParentListItem::measure(const SDL_Rect& availableExtent)
{
    // Run the normal measure step (measures our children and sets our 
    // scaledExtent).
    Widget::measure(availableExtent);

    // Since our child container might expand or collapse, adjust our height to 
    // match.
    logicalExtent.h = childListItemContainer.getLogicalExtent().h;
}

} // End namespace ResourceImporter
} // End namespace AM
