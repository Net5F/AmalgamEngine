#pragma once

#include "AUI/Window.h"
#include "AUI/Image.h"
#include "AUI/Text.h"
#include "AUI/Checkbox.h"
#include "MainTextInput.h"
#include "MainButton.h"
#include "LibraryItemData.h"

namespace AM
{
struct BoundingBox;

namespace ResourceImporter
{
class DataModel;
class LibraryWindow;
class LibraryListItem;

/**
 * The properties window shown when the user loads an animation from the Library.
 * Allows the user to edit the active animation's properties.
 */
class AnimationPropertiesWindow : public AUI::Window
{
public:
    //-------------------------------------------------------------------------
    // Public interface
    //-------------------------------------------------------------------------
    AnimationPropertiesWindow(DataModel& ineDataModel,
                           LibraryWindow& inLibraryWindow);

    //-------------------------------------------------------------------------
    // Public child widgets
    //-------------------------------------------------------------------------
    /** All fields below directly match a data field in the EditorAnimation 
        class. See fields below for more info. */
    AUI::Text nameLabel;
    MainTextInput nameInput;

    AUI::Text frameCountLabel;
    MainTextInput frameCountInput;

    AUI::Text fpsLabel;
    MainTextInput fpsInput;

    AUI::Text boundingBoxLabel;
    AUI::Text boundingBoxNameLabel;
    MainButton boundingBoxButton;

    AUI::Text minXLabel;
    MainTextInput minXInput;

    AUI::Text minYLabel;
    MainTextInput minYInput;

    AUI::Text minZLabel;
    MainTextInput minZInput;

    AUI::Text maxXLabel;
    MainTextInput maxXInput;

    AUI::Text maxYLabel;
    MainTextInput maxYInput;

    AUI::Text maxZLabel;
    MainTextInput maxZInput;

    AUI::Text collisionEnabledLabel;
    AUI::Checkbox collisionEnabledInput;

private:
    /**
     * If the new active item is a animation, loads it's data into this panel.
     */
    void onActiveLibraryItemChanged(const LibraryItemData& newActiveItem);

    /**
     * (If active animation was removed) Sets this panel back to its default 
     * state.
     */
    void onAnimationRemoved(AnimationID animationID);

    /**
     * (If ID matches active animation) Updates this panel with the active 
     * animation's new properties.
     */
    void onAnimationDisplayNameChanged(AnimationID animationID,
                                       const std::string& newDisplayName);
    void onAnimationFrameCountChanged(AnimationID animationID,
                                      Uint8 newFrameCount);
    void onAnimationFpsChanged(AnimationID animationID,
                               Uint8 newFps);
    void onAnimationModelBoundsIDChanged(AnimationID animationID,
                                         BoundingBoxID newModelBoundsID);
    void onAnimationCustomModelBoundsChanged(
        AnimationID animationID, const BoundingBox& newCustomModelBounds);
    void onAnimationCollisionEnabledChanged(AnimationID animationID,
                                            bool newCollisionEnabled);

    /**
     * Updates boundingBoxButton to show whether the list item is assignable.
     */
    void onLibraryListItemSelected(const LibraryListItem& selectedItem);
    void onLibraryListItemDeselected(const LibraryListItem& deselectedItem);

    /**
     * Enables or disables the min/max bounds fields.
     */
    void setBoundsFieldsEnabled(bool isEnabled);

    /** Used while setting user-inputted animation data. */
    DataModel& dataModel;

    /** Used to get the currently selected list item. */
    LibraryWindow& libraryWindow;

    /** The active animation's ID. */
    AnimationID activeAnimationID;

    /**
     * Converts the given float to a string with 3 decimals of precision.
     */
    std::string toRoundedString(float value);

    /** The below functions are all for validating and saving the user's data
        when the text is committed. */
    void saveName();
    void saveModelBoundsID();
    void saveMinX();
    void saveMinY();
    void saveMinZ();
    void saveMaxX();
    void saveMaxY();
    void saveMaxZ();
    void saveCollisionEnabled();

    /** The below floats save the committed values, so we can revert to them
        if the user inputs invalid characters. */
    float committedMinX;
    float committedMinY;
    float committedMinZ;
    float committedMaxX;
    float committedMaxY;
    float committedMaxZ;

    //-------------------------------------------------------------------------
    // Private child widgets
    //-------------------------------------------------------------------------
    AUI::Image backgroundImage;

    AUI::Image headerImage;

    AUI::Text windowLabel;
};

} // End namespace ResourceImporter
} // End namespace AM
