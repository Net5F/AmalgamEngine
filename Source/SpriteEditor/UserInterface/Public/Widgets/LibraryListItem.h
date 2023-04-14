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
 * The rendering order for this widget's children is:
 *   Background: selectedImage, activeImage
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

    /**
     * Activates this widget and calls onActivated.
     *
     * If this widget is already active, does nothing.
     *
     * Disables hovering. Any active hover state will be removed.
     *
     * Note: This widget activates itself when double clicked. This function
     *       just exists in case you need to do it programatically.
     */
    void activate();

    /**
     * Deactivates this widget and calls onDeactivated.
     *
     * If this widget isn't active, does nothing.
     *
     * Note: This widget doesn't deactivate itself. The context that is
     *       managing this widget must detect when the widget should be
     *       deactivated and call this method.
     */
    void deactivate();

    bool getIsHovered() const;
    bool getIsSelected() const;
    bool getIsActive() const;

    /**
     * Sets the left padding. Used to define the visual hierarchy in the list.
     */
    void setLeftPadding(int inLeftPadding);

    //-------------------------------------------------------------------------
    // Public child widgets
    //-------------------------------------------------------------------------
    /** Background image, hovered state. */
    AUI::Image hoveredImage;
    /** Background image, selected state. */
    AUI::Image selectedImage;
    /** Background image, active state. */
    AUI::Image activeImage;

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

    /**
     * @param inOnDeactivated  A callback that expects a pointer to the
     *                         widget that was deactivated.
     */
    void setOnDeactivated(std::function<void(LibraryListItem*)> inOnDeactivated);

    //-------------------------------------------------------------------------
    // Base class overrides
    //-------------------------------------------------------------------------
    AUI::EventResult onMouseDown(AUI::MouseButtonType buttonType,
                            const SDL_Point& cursorPosition) override;

    AUI::EventResult onMouseDoubleClick(AUI::MouseButtonType buttonType,
                                   const SDL_Point& cursorPosition) override;

    void onMouseEnter() override;

    void onMouseLeave() override;

private:
    /** Sets isHovered and updates the visibility of hoveredImage. */
    void setIsHovered(bool inIsHovered);
    /** Sets isSelected and updates the visibility of selectedImage. */
    void setIsSelected(bool inIsSelected);
    /** Sets isActive and updates the visibility of activeImage. */
    void setIsActive(bool inIsActive);

    std::function<void(LibraryListItem*)> onSelected;
    std::function<void(LibraryListItem*)> onDeselected;
    std::function<void(LibraryListItem*)> onActivated;
    std::function<void(LibraryListItem*)> onDeactivated;

    /** Tracks whether the mouse is currently hovering over this widget. */
    bool isHovered;

    /** Tracks whether this widget is currently selected. */
    bool isSelected;

    /** Tracks whether this widget is currently active. */
    bool isActive;
};

} // End namespace SpriteEditor
} // End namespace AM
