#pragma once

#include "AUI/Widget.h"
#include "AUI/Image.h"
#include "AUI/Text.h"
#include <functional>

namespace AM
{
namespace ResourceImporter
{
/**
 * A selectable list item, used in the animation elements window.
 *
 * Interactions:
 *   Mouse-over to hover.
 *   Click once to "select", putting this widget into a "selected" state.
 *
 * The rendering order for this widget's children is:
 *   Background: hoveredImage, selectedImage
 *   Foreground: text
 */
class AnimationElementsListItem : public AUI::Widget
{
public:
    //-------------------------------------------------------------------------
    // Public definitions
    //-------------------------------------------------------------------------
    /**
     * Used to track this widget's visual and logical state.
     */
    enum class State { Normal, Hovered, Selected, Disabled };

    //-------------------------------------------------------------------------
    // Public interface
    //-------------------------------------------------------------------------
    AnimationElementsListItem(const SDL_Rect& inLogicalExtent,
                              const std::string& inText,
                              const std::string& inDebugName
                              = "AnimationElementsListItem");

    /**
     * Enabled this list item, allowing it to respond to user input.
     */
    void enable();

    /**
     * Disables this list item, causing it to ignore user input and render with
     * grey text.
     */
    void disable();

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

    State getCurrentState() const;

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

    /** List item text. */
    AUI::Text text;

    //-------------------------------------------------------------------------
    // Callback registration
    //-------------------------------------------------------------------------
    /**
     * @param inOnSelected A callback that expects a pointer to the widget
     *                     that was selected.
     */
    void setOnSelected(
        std::function<void(AnimationElementsListItem*)> inOnSelected);

    /**
     * @param inOnDeselected A callback that expects a pointer to the
     *                       widget that was deselected.
     */
    void setOnDeselected(
        std::function<void(AnimationElementsListItem*)> inOnDeselected);

    //-------------------------------------------------------------------------
    // Base class overrides
    //-------------------------------------------------------------------------
    AUI::EventResult onMouseDown(AUI::MouseButtonType buttonType,
                                 const SDL_Point& cursorPosition) override;

    AUI::EventResult
        onMouseDoubleClick(AUI::MouseButtonType buttonType,
                           const SDL_Point& cursorPosition) override;

    void onMouseEnter() override;

    void onMouseLeave() override;

protected:
    /**
     * Sets currentState and updates child widget visibility.
     */
    void setCurrentState(State inState);

    std::function<void(AnimationElementsListItem*)> onSelected;
    std::function<void(AnimationElementsListItem*)> onDeselected;

    /** Tracks this button's current visual and logical state. */
    State currentState;
};

} // End namespace ResourceImporter
} // End namespace AM
