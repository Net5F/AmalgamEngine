#pragma once

#include "LibraryListItem.h"
#include "LibraryCollapsibleContainer.h"
#include <SDL_rect.h>

namespace AM
{
namespace SpriteEditor
{
/**
 * A collapsible container used for the sprite sheets in the library window on 
 * the main screen.
 *
 * Derives from LibraryListItem for hover/select functionality and library 
 * list info. Has a child CategoryContainer to hold the sprites that belong 
 * to this sheet.
 */
class SpriteSheetListItem : public LibraryListItem 
{
public:
    SpriteSheetListItem(const std::string& inHeaderText,
                         const std::string& inDebugName
                             = "SpriteSheetListItem");

    //-------------------------------------------------------------------------
    // Base class overrides
    //-------------------------------------------------------------------------
    AUI::EventResult onMouseDown(AUI::MouseButtonType buttonType,
                            const SDL_Point& cursorPosition) override;

    AUI::EventResult onMouseDoubleClick(AUI::MouseButtonType buttonType,
                                   const SDL_Point& cursorPosition) override;

    AUI::EventResult onMouseMove(const SDL_Point& cursorPosition) override;

    void onMouseEnter() override;

    void onMouseLeave() override;

    //-------------------------------------------------------------------------
    // Public child widgets
    //-------------------------------------------------------------------------
    /** Holds list item children for the sprites in this sheet. */
    LibraryCollapsibleContainer spriteListItemContainer;
};

} // End namespace SpriteEditor
} // End namespace AM
