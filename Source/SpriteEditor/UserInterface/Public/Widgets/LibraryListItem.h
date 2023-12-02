#pragma once

#include "AUI/Widget.h"
#include "AUI/Image.h"
#include "AUI/Text.h"

namespace AM
{
namespace SpriteEditor
{
/**
 * A selectable list item, used in the library window.
 *
 * Interactions:
 *   Mouse-over to hover.
 *   Click once to "select", putting this widget into a "selected" state.
 *   Double-click to "activate". Does not change internal state.
 *
 * The rendering order for this widget's children is:
 *   Background: hoveredImage, selectedImage
 *   Foreground: text
 */
class LibraryListItem : public AUI::Widget
{
public:
    //-------------------------------------------------------------------------
    // Public interface
    //-------------------------------------------------------------------------
    LibraryListItem(const std::string& inText,
                    const std::string& inDebugName
                             = "LibraryListItem");

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

    bool getIsHovered() const;
    bool getIsSelected() const;

    /**
     * Sets the left padding. Used to define the visual hierarchy in the list.
     */
    void setLeftPadding(int inLeftPadding);

    /**
     * The types of list items that we hold in the library.
     */
    enum Type
    {
        SpriteSheet,
        Sprite,
        Floor,
        FloorCovering,
        Wall,
        Object,
        IconSheet,
        Icon,
        Count,
        None
    };
    Type type{};

    /** Alongside type, associates this list item with the model data that it 
        represents.
        Note: For sprite sets, this can be cast to Uint16. */
    int ID{0};

    //-------------------------------------------------------------------------
    // Public child widgets
    //-------------------------------------------------------------------------
    /** Background image, hovered state. */
    AUI::Image hoveredImage;
    /** Background image, selected state. */
    AUI::Image selectedImage;

    /** List item text. */
    AUI::Text text;

    //-------------------------------------------------------------------------
    // Callback registration
    //-------------------------------------------------------------------------
    /**
     * @param inOnSelected  A callback that expects a pointer to the widget
     *                      that was selected.
     */
    void setOnSelected(std::function<void(LibraryListItem*)> inOnSelected);

    /**
     * @param inOnDeselected  A callback that expects a pointer to the
     *                        widget that was deselected.
     */
    void setOnDeselected(std::function<void(LibraryListItem*)> inOnDeselected);

    /**
     * @param inOnActivated  A callback that expects a pointer to the widget
     *                       that was activated.
     */
    void setOnActivated(std::function<void(LibraryListItem*)> inOnActivated);

    //-------------------------------------------------------------------------
    // Base class overrides
    //-------------------------------------------------------------------------
    AUI::EventResult onMouseDown(AUI::MouseButtonType buttonType,
                            const SDL_Point& cursorPosition) override;

    AUI::EventResult onMouseDoubleClick(AUI::MouseButtonType buttonType,
                                   const SDL_Point& cursorPosition) override;

    void onMouseEnter() override;

    void onMouseLeave() override;

protected:
    /** Sets isHovered and updates the visibility of hoveredImage. */
    void setIsHovered(bool inIsHovered);
    /** Sets isSelected and updates the visibility of selectedImage. */
    void setIsSelected(bool inIsSelected);

    std::function<void(LibraryListItem*)> onSelected;
    std::function<void(LibraryListItem*)> onDeselected;
    std::function<void(LibraryListItem*)> onActivated;

    /** Tracks whether the mouse is currently hovering over this widget. */
    bool isHovered;

    /** Tracks whether this widget is currently selected. */
    bool isSelected;
};

} // End namespace SpriteEditor
} // End namespace AM
