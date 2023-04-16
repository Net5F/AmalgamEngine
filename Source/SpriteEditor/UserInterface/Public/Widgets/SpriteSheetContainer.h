#pragma once

#include "CategoryContainer.h"
#include <SDL_rect.h>

namespace AM
{
namespace SpriteEditor
{
/**
 * A collapsible container used for the sprite sheets in the library window on 
 * the main screen.
 *
 * Derives from CategoryContainer, adds "selectable" functionality so we can 
 * select the sprite sheets to e.g. delete them.
 */
class SpriteSheetContainer : public CategoryContainer
{
public:
    SpriteSheetContainer(const std::string& inHeaderText,
                         const std::string& inDebugName
                             = "SpriteSheetContainer");

    /**
     * Selects this widget and calls onSelected.
     *
     * If this widget is already selected or isSelectable == false, does
     * nothing.
     *
     * Note: This widget selects itself when clicked. This function just
     *       exists in case you need to do it programatically.
     */
    void select();

    /**
     * Deselects this widget and calls onDeselected.
     *
     * If this widget isn't selected, does nothing.
     *
     * Note: This widget doesn't deselect itself. The context that is
     *       managing this widget must detect when the widget should be
     *       deactivated and call this method.
     */
    void deselect();

    bool getIsSelected() const;

    //-------------------------------------------------------------------------
    // Public child widgets
    //-------------------------------------------------------------------------
    /** Background image, hovered state. */
    AUI::Image hoveredImage;
    /** Background image, selected state. */
    AUI::Image selectedImage;

    //-------------------------------------------------------------------------
    // Callback registration
    //-------------------------------------------------------------------------
    /**
     * @param inOnSelected  A callback that expects a pointer to the widget
     *                      that was selected.
     */
    void setOnSelected(std::function<void(SpriteSheetContainer*)> inOnSelected);

    /**
     * @param inOnDeselected  A callback that expects a pointer to the
     *                        widget that was deselected.
     */
    void setOnDeselected(std::function<void(SpriteSheetContainer*)> inOnDeselected);


    //-------------------------------------------------------------------------
    // Base class overrides
    //-------------------------------------------------------------------------
    AUI::EventResult onMouseDown(AUI::MouseButtonType buttonType,
                            const SDL_Point& cursorPosition) override;

    AUI::EventResult onMouseDoubleClick(AUI::MouseButtonType buttonType,
                                   const SDL_Point& cursorPosition) override;

    AUI::EventResult onMouseMove(const SDL_Point& cursorPosition) override;

    void onMouseLeave() override;

private:
    /** Sets isHovered and updates the visibility of hoveredImage. */
    void setIsHovered(bool inIsHovered);
    /** Sets isSelected and updates the visibility of selectedImage. */
    void setIsSelected(bool inIsSelected);

    std::function<void(SpriteSheetContainer*)> onSelected;
    std::function<void(SpriteSheetContainer*)> onDeselected;

    /** Tracks whether the mouse is currently hovering over this widget. */
    bool isHovered;

    /** Tracks whether this widget is currently selected. */
    bool isSelected;

    /** The extent around the expand/collapse arrow that is clickable.
        Clicking outside of this extent will cause it to be selected instead. */
    SDL_Rect clickableExtent;
};

} // End namespace SpriteEditor
} // End namespace AM
