#pragma once

#include "AnimationElementsListItem.h"
#include "LibraryItemData.h"
#include "AUI/Window.h"
#include "AUI/Image.h"
#include "AUI/Text.h"
#include "AUI/Button.h"
#include <functional>

namespace AM
{
struct BoundingBox;

namespace ResourceImporter
{
class MainScreen;
class DataModel;

/**
 * The elements window shown when the user loads an animation from the Library.
 * Allows the user to select an element, and to add or remove elements.
 */
class AnimationElementsWindow : public AUI::Window
{
public:
    //-------------------------------------------------------------------------
    // Public definitions
    //-------------------------------------------------------------------------
    /**
     * The types of elements items that we hold in this window.
     */
    enum ElementType {
        BoundingBox,
        EntityAlignmentAnchor,
        Count,
        None
    };

    //-------------------------------------------------------------------------
    // Public interface
    //-------------------------------------------------------------------------
    AnimationElementsWindow(MainScreen& inScreen, DataModel& ineDataModel);

    //-------------------------------------------------------------------------
    // Public child widgets
    //-------------------------------------------------------------------------
    AnimationElementsListItem boundingBoxListItem;

    AnimationElementsListItem entityAlignmentAnchorListItem;

    //-------------------------------------------------------------------------
    // Callback registration
    //-------------------------------------------------------------------------
    /**
     * @param inOnListItemSelected A callback that expects an enum value of the
     *                             element type that was selected.
     */
    void setOnListItemSelected(
        std::function<void(ElementType)> inOnListItemSelected);

private:
    /**
     * If the new active item is a animation, loads it's data into this panel.
     */
    void onActiveLibraryItemChanged(const LibraryItemData& newActiveItem);

    /**
     * (If ID matches active animation) Updates this panel with the active 
     * animation's new properties.
     */
    void onAnimationEntityAlignmentAnchorChanged(
        AnimationID animationID,
        const std::optional<Vector3>& newEntityAlignmentAnchor);

    /**
     * (If active animation was removed) Sets this panel back to its default 
     * state.
     */
    void onAnimationRemoved(AnimationID animationID);

    /**
     * Updates this widgets graphical state to match the given anchor state.
     */
    void updateAnchorListItemState(
        const std::optional<Vector3>& entityAlignmentAnchor);

    /** Used to open the confirmation dialog when saving a bounding box. */
    MainScreen& mainScreen;

    /** Used while setting user-inputted animation data. */
    DataModel& dataModel;

    std::function<void(ElementType selectedType)> onListItemSelected;

    /** The active animation's ID. */
    AnimationID activeAnimationID;

    //-------------------------------------------------------------------------
    // Private child widgets
    //-------------------------------------------------------------------------
    AUI::Image backgroundImage;

    AUI::Image headerImage;

    AUI::Text windowLabel;

    AUI::Button addEntityAlignmentAnchorButton;

    AUI::Button remEntityAlignmentAnchorButton;
};

} // End namespace ResourceImporter
} // End namespace AM
