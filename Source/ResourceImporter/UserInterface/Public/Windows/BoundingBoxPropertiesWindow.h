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

/**
 * The properties window shown when the user loads a bounding box from the 
 * Library.
 * Allows the user to edit the active bounding box's properties.
 */
class BoundingBoxPropertiesWindow : public AUI::Window
{
public:
    //-------------------------------------------------------------------------
    // Public interface
    //-------------------------------------------------------------------------
    BoundingBoxPropertiesWindow(DataModel& ineDataModel,
                           const LibraryWindow& inLibraryWindow);

    //-------------------------------------------------------------------------
    // Public child widgets
    //-------------------------------------------------------------------------
    /** All fields below directly match a data field in the EditorSprite class.
        See displayName, collisionEnabled, and modelBounds fields for more
        information. */
    AUI::Text nameLabel;
    MainTextInput nameInput;

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

private:
    /**
     * If the new active item is a bounding box, loads it's data into this panel.
     */
    void onActiveLibraryItemChanged(const LibraryItemData& newActiveItem);

    /**
     * (If active bounding box was removed) Sets this panel back to its default 
     * state.
     */
    void onBoundingBoxRemoved(BoundingBoxID boundingBoxID);

    /**
     * (If ID matches active bounding box) Updates this panel with the active 
     * bounding box's new properties.
     */
    void onBoundingBoxDisplayNameChanged(BoundingBoxID boundingBoxID,
                                         const std::string& newDisplayName);
    void onBoundingBoxBoundsChanged(BoundingBoxID boundingBoxID,
                                    const BoundingBox& newBounds);

    /** Used while setting user-inputted sprite data. */
    DataModel& dataModel;

    /** Used to get the currently selected list item. */
    const LibraryWindow& libraryWindow;

    /** The active bounding box's ID. */
    BoundingBoxID activeBoundingBoxID;

    /**
     * Converts the given float to a string with 3 decimals of precision.
     */
    std::string toRoundedString(float value);

    /** The below functions are all for validating and saving the user's data
        when the text is committed. */
    void saveName();
    void saveMinX();
    void saveMinY();
    void saveMinZ();
    void saveMaxX();
    void saveMaxY();
    void saveMaxZ();

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
