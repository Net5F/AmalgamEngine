#pragma once

#include "LibraryListItem.h"
#include "LibraryCollapsibleContainer.h"
#include <SDL_rect.h>

namespace AM
{
namespace ResourceImporter
{
/**
 * A collapsible container used for the top-level list items in the library
 * window on the main screen, such as sprite and icon sheets which contain a
 * list of sprites or items.
 *
 * Derives from LibraryListItem for hover/select functionality and library
 * list info.
 */
class ParentListItem : public LibraryListItem
{
public:
    ParentListItem(const std::string& inHeaderText,
                   const std::string& inDebugName = "ParentListItem");

    //-------------------------------------------------------------------------
    // Base class overrides
    //-------------------------------------------------------------------------
    AUI::EventResult onMouseDown(AUI::MouseButtonType buttonType,
                                 const SDL_Point& cursorPosition) override;

    AUI::EventResult
        onMouseDoubleClick(AUI::MouseButtonType buttonType,
                           const SDL_Point& cursorPosition) override;

    AUI::EventResult onMouseMove(const SDL_Point& cursorPosition) override;

    void onMouseEnter() override;

    void onMouseLeave() override;

    //-------------------------------------------------------------------------
    // Public child widgets
    //-------------------------------------------------------------------------
    /** Holds list item children. */
    LibraryCollapsibleContainer childListItemContainer;
};

} // End namespace ResourceImporter
} // End namespace AM
